#include "game.h"
#include "utilities.h"
// Standard Includes for MPI, C and OS calls
#include <mpi.h>

// C++ standard I/O and library includes
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// C++ standard library using statements
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

const int CHUNK_SIZE = 8;

struct game_results {
	game_state game_board; 		// initial game board that converts char buffer to game's grid
	move solution[IDIM*JDIM]; 	// move history that yields a solution to the puzzle
	bool found = false; 		// solution found
	int size = 0;				// number of steps used in solution
	int solved_by;				// processor that worked on the puzzle
};

void search(unsigned char buf[], game_results results[], int batch_size, int rank) {
	for (int i = 0; i < batch_size; ++i) {
		results[i].game_board.Init(&buf[i*IDIM*JDIM]);
		//results[i].start_board.Init(buf[i]);
		// Search for a solution to the puzzle
		results[i].found = depthFirstSearch(results[i].game_board,
											results[i].size,
											results[i].solution);
		results[i].solved_by = rank;
	}
}

class Server {
	public:
	vector<string> puzzles; // will essentially act as the bag in the bag of tasks
	int NUM_GAMES;
	ifstream input{argv[1], ios::in};	// Input case filename
	ofstream output{argv[2], ios::out}; // Output case filename
	Server(int argc, char *argv[]) {
	// Check to make sure the server can run
		if (argc != 3) {
			cerr << "two arguments please!" << endl;
			MPI_Abort(MPI_COMM_WORLD, -1);
		}
		this.input >> this.NUM_GAMES; // get the number of games from first line of the input file
		this.puzzles.reserve(this.NUM_GAMES);
		// initialize a vector with all of the puzzles saved as a string
		for (int i = 0; i < this.NUM_GAMES; ++i) { // for each game in file...
			string input_string;
			this.input >> input_string; // reads line by line
			if (input_string.size() != IDIM*JDIM) {
				cerr << "something wrong in input file format!" << endl;
				MPI_Abort(MPI_COMM_WORLD, -1);
			}
			this.puzzles.push_back(input_string);
		}
	}

	void record_output(vector<game_results> &games) { // may need this argument: std::ostream &output, 
		int num_sol = games.size();
		for(int k = 0; k < num_sol) {
			this.output << "found solution = " << endl;
			games[k].game_board.Print(this.output);
			for (int i = 0; i < games[k].size; ++i) {
				games[k].game_board.makeMove(games[k].solution[i]);
				this.output << "-->" << endl;
				games[k].game_board.Print(this.output);
			}
			this.output << "solved" << endl;
		}
		cout << "found " << num_sol << " solutions" << endl;
	}

	void puzzle_copy(unsigned char buffer[], int batch_size) {
		for(int j = 0; j < batch_size; ++j) {
			// assumes unsigned chars in C string are each 1 byte
			memcpy(&buffer[j*IDIM*JDIM], this.puzzles[0].c_str(), IDIM*JDIM);
			this.puzzles.erase(this.puzzles.begin());
		}
	}

	void solve_puzzles(int num_proc) {
		vector<game_results> solved_puzzles;
		int SERVER_CHUNK = 0.5*CHUNK_SIZE;
		bool chunk_size_flag = false;
		// unsigned char buf[num_proc-1][IDIM*JDIM*CHUNK_SIZE];
		MPI_Request send_req[num_proc-1];
		MPI_Request recv_req[num_proc-1];
		int stop_proc_tag = -1;
		int more_work_tag = -10;
		int ret_result_tag = -100;
		unsigned char buf[IDIM*JDIM*CHUNK_SIZE];
		for(int i = 0; i < num_proc-1; ++i) {
			this.puzzle_copy(buf, CHUNK_SIZE);
			MPI_Isend(buf, IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, i+1, MPI_ANY_TAG, MPI_COMM_WORLD, &send_req[i]);
			MPI_Wait(&send_req[i]);
		}
		/* MPI_IScatter(&buf, IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR,
					IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD); */
		// unsigned char buf[CHUNK_SIZE] = new unsigned char[IDIM*JDIM]; // new buffer for this iteration's game
		while (!this.puzzles.empty()) {
		// ########################### SERVER DOING TASKS ############################
			if(chunk_size_flag)
				SERVER_CHUNK = this.puzzles.size();
			game_results results[SERVER_CHUNK];
			unsigned char server_buf[IDIM*JDIM*SERVER_CHUNK];
			this.puzzle_copy(server_buf, SERVER_CHUNK);
			search(server_buf, results, SERVER_CHUNK, 0);
			for(int i=0; i < SERVER_CHUNK; ++i) {
				if (results[i].found)
					solved_puzzles.push_back(results[i]);
			}
		// ################## SERVER CHECKING FOR COMPLETED TASKS FROM CLIENTS ####################
			bool flag = 0;
			if(this.puzzles.size() < CHUNK_SIZE)
				chunk_size_flag = true;
			for(int i = 0; i < num_proc-1) {
				MPI_Iprobe(i+1, ret_result_tag, MPI_COMM_WORLD, &flag);
				if(flag) {
					game_results rec_buffer[CHUNK_SIZE];
					// second parameter count is int in docs, so may need to broadcast it
					MPI_Irecv(rec_buffer, CHUNK_SIZE*sizeof(game_results), MPI_BYTE, i+1, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_req[i]);
					MPI_Wait(&recv_req[i]);
					for(int k = 0; k < CHUNK_SIZE; ++k) {
						if(rec_buffer[k].found)
							solved_puzzles.push_back(rec_buffer[k]);
					}
					if(!chunk_size_flag) {
						unsigned char new_buf[IDIM*JDIM*CHUNK_SIZE];
						// had this for loop combined with the one above, but it require more conditionals
						this.puzzle_copy(new_buf, CHUNK_SIZE);
						MPI_ISend(new_buf, IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, i+1, more_work_tag, MPI_COMM_WORLD, &send_req[i]);
						MPI_Wait(&send_req[i]);
						if(this.puzzles.size() < CHUNK_SIZE)
							chunk_size_flag = true;
					}
					else
						MPI_Send(100, 1, MPI_INT, i+1, stop_proc_tag, MPI_COMM_WORLD);
				}
			}
		}
		// DELETE buf here
		this.record_output(solved_puzzles);
	}
};


// Put the code for the client here
void Client(int rank){
	// send buf[] to the client processor and use game_board.Init() on the Client to
	// initialize game board. The result that will need to be sent back will either be
	// the moves required to solve the game or an indication that the game was not solvable.
	bool exit_flag = false;
	bool msg_flag = false;
	int stop_proc_tag = -1;
	int more_work_tag = -10;
	int ret_result_tag = -100;
	MPI_Request send_req;
	MPI_Request recv_req;
	unsigned char buf[IDIM*JDIM*CHUNK_SIZE];
	MPI_Irecv(buf, IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_req);
	MPI_Wait(&recv_req);

	while(!exit_flag){
		game_results results[CHUNK_SIZE];
		search(buf, results, CHUNK_SIZE, rank);
		MPI_ISend(results, CHUNK_SIZE*sizeof(game_results), MPI_BYTE, 0, ret_result_tag, MPI_COMM_WORLD, &send_req);
		// if MPI receive value is specific status, break
		MPI_Wait(&send_req);
		MPI_Iprobe(0, stop_proc_tag, MPI_COMM_WORLD, &exit_flag);
		if(!exit_flag) {
			MPI_Irecv(buf, IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, 0, more_work_tag, MPI_COMM_WORLD, &recv_req);
			MPI_Wait(&recv_req);
		}
	}
}

int main(int argc, char *argv[]) {
	// This is a utility routine that installs an alarm to kill off this
	// process if it runs to long.  This will prevent jobs from hanging
	// on the queue keeping others from getting their work done.
	chopsigs_();
	// All MPI programs must call this function
	MPI_Init(&argc, &argv);
	int myId;
	int numProcessors;
	/* Get the number of processors and my processor identification */
	MPI_Comm_size(MPI_COMM_WORLD, &numProcessors);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);
	if (myId == 0) {// Processor 0 runs the server code
		get_timer(); // zero the timer
		Server server_process(argc, argv);
		server_process.solve_puzzles(numProcessors);
		// Measure the running time of the server
		cout << "execution time = " << get_timer() << " seconds." << endl;
	}
	else {// all other processors run the client code.
		Client(myId);
	}
	// All MPI programs must call this before exiting
	MPI_Finalize();
}
