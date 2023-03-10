#include "game.h"
#include "utilities.h"
// Standard Includes for MPI, C and OS calls
#include <mpi.h>

// C++ standard I/O and library includes
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

// C++ standard library using statements
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;

const int CHUNK_SIZE = 16;
int STOP_PROC_TAG = 5;
int MORE_WORK_TAG = 10;
int RET_RESULT_TAG = 100;

struct game_results {
	game_state game_board; 		// initial game board that converts char buffer to game's grid
	move solution[IDIM*JDIM]; 	// move history that yields a solution to the puzzle
	bool found = false; 		// solution found
	int size = 0;				// number of steps used in solution
	int solved_by;				// processor that worked on the puzzle
};

struct server_storage {
	vector<string> puzzles; // will essentially act as the bag in the bag of tasks
	int NUM_GAMES;
	vector<game_results> solved_puzzles;
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

void record_output(server_storage &serv, ofstream &output) {
// called at the end of solve_puzzles() to write all intelligent puzzles to the output file
	int num_sol = serv.solved_puzzles.size();
	for(int k = 0; k < num_sol; ++k) {
		output << "found solution = " << endl;
		serv.solved_puzzles[k].game_board.Print(output);
		for (int i = 0; i < serv.solved_puzzles[k].size; ++i) {
			serv.solved_puzzles[k].game_board.makeMove(serv.solved_puzzles[k].solution[i]);
			output << "-->" << endl;
			serv.solved_puzzles[k].game_board.Print(output);
		}
		output << "solved" << endl;
	}
	cout << "found " << num_sol << " solutions" << endl;
	output.close();
}

void puzzle_copy(unsigned char buffer[], int batch_size, server_storage &serv) {
// copies batch_size puzzle strings into the buffer reference from Server::puzzles
	for(int j = 0; j < batch_size; ++j) {
		// assumes unsigned chars in C string are each 1 byte
		std::memcpy(&buffer[j*IDIM*JDIM], serv.puzzles[0].c_str(), IDIM*JDIM);
		serv.puzzles.erase(serv.puzzles.begin());
	}
}

void solve_locally(server_storage &serv, int batch_size) {
	game_results results[batch_size];
	unsigned char server_buf[IDIM*JDIM*batch_size];
	puzzle_copy(server_buf, batch_size, serv);
	search(server_buf, results, batch_size, 0);
	for(int i=0; i < batch_size; ++i) {
		if (results[i].found)
			serv.solved_puzzles.push_back(results[i]);
	}
}


void Server(int argc, char *argv[], int num_proc) {
	ifstream input(argv[1],ios::in); 	// Input case filename
	ofstream output(argv[2],ios::out) ; // Output case filename
	if (argc != 3) {
		cerr << "two arguments please!" << endl;
		MPI_Abort(MPI_COMM_WORLD, -1);
	}
	server_storage serv;
	input >> serv.NUM_GAMES; // get the number of games from first line of the input file
	// initialize a vector with all of the puzzles saved as a string
	serv.puzzles.reserve(serv.NUM_GAMES);
	for (int i = 0; i < serv.NUM_GAMES; ++i) { // for each game in file...
		string input_string;
		input >> input_string; // reads line by line
		// each puzzle given by IDIM*JDIM characters per line
		if (input_string.size() != IDIM*JDIM) {
			cerr << "something wrong in input file format!" << endl;
			MPI_Abort(MPI_COMM_WORLD, -1);
		}
		serv.puzzles.push_back(input_string);
	}
	if(num_proc == 1){
		solve_locally(serv, serv.NUM_GAMES);
		record_output(serv, output);
		return;
	}
	// cout << "ENTERING MAIN FUNCTION OF THE SERVER" << endl;
	// vector<game_results> solved_puzzles;
	int SERVER_CHUNK = 0.5*CHUNK_SIZE;
	bool chunk_size_flag = false;
	int num_puzzles = serv.NUM_GAMES;
	MPI_Request send_req[num_proc-1];
	MPI_Request recv_req[num_proc-1];
	unsigned char send_buf[num_proc-1][IDIM*JDIM*CHUNK_SIZE];
	game_results recv_buffer[num_proc-1][CHUNK_SIZE];
	int idle_proc;
	int finished_proc = 0;
	for(int i = 0; i < num_proc-1; ++i) {
		puzzle_copy(send_buf[i], CHUNK_SIZE, serv);
		MPI_Isend(&send_buf[i], IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, i+1, 1, MPI_COMM_WORLD, &send_req[i]);
		MPI_Irecv(&recv_buffer[i], CHUNK_SIZE*sizeof(game_results), MPI_BYTE, MPI_ANY_SOURCE, RET_RESULT_TAG, MPI_COMM_WORLD, &recv_req[i]);
		//MPI_Wait(&send_req[i], MPI_STATUS_IGNORE);
		num_puzzles -= CHUNK_SIZE;
	}
	// cout << "COMPLETED INITIAL DISTRIBUTION OF WORK; PUZZLES REMAINING: " << num_puzzles << endl;
	// unsigned char buf[CHUNK_SIZE] = new unsigned char[IDIM*JDIM]; // new buffer for this iteration's game
	while (num_puzzles >= 0) {
	// ########################### SERVER DOING TASKS ############################
		/*if(chunk_size_flag && num_puzzles != 0) {
			cout << "ENTERED FINAL STAGE OF SERVER WHERE IT COMPLETES THE LAST PUZZLES.";
			cout << endl << "PUZZLES REMAINING: " << num_puzzles << endl;
			SERVER_CHUNK = serv.puzzles.size();
			solve_locally(serv, SERVER_CHUNK);
			num_puzzles -= SERVER_CHUNK;
		}*/
	// ################## SERVER CHECKING FOR COMPLETED TASKS FROM CLIENTS ####################
		if(num_puzzles == 0 && finished_proc == num_proc-1)
			break;
		chunk_size_flag = (num_puzzles < CHUNK_SIZE);
		//MPI_Iprobe(i+1, RET_RESULT_TAG, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
		//game_results recv_buffer[CHUNK_SIZE];
		MPI_Request recv_request;
		MPI_Status recv_status;
		int test_flag = false;
		int idle_proc;
		MPI_Testany(num_proc-1, recv_req, &idle_proc, &test_flag, &recv_status);
		// MPI_Iprobe(MPI_ANY_SOURCE, RET_RESULT_TAG, MPI_COMM_WORLD, &test_flag, &recv_status);
		//MPI_Irecv(&recv_buffer, CHUNK_SIZE*sizeof(game_results), MPI_BYTE, MPI_ANY_SOURCE, RET_RESULT_TAG, MPI_COMM_WORLD, &recv_request);
		//MPI_Request_get_status(recv_request, &test_flag, &recv_status);
		//MPI_Wait(&recv_request, &recv_status);
		if(test_flag) {
			idle_proc += 1;
			MPI_Wait(&recv_req[idle_proc-1], MPI_STATUS_IGNORE);
			//recv_req[idle_proc-1] = 
			// cout << "COMPLETED MPI_Irecv ON THE SERVER SIDE" << endl;
			// cout << "SANITY CHECK OF RECEIVING PROCESSOR ID: " << idle_proc << endl;
			for(int k = 0; k < CHUNK_SIZE; ++k) {
				if(recv_buffer[idle_proc-1][k].found)
					serv.solved_puzzles.push_back(recv_buffer[idle_proc-1][k]);
			}
			// MPI_Irecv(recv_buffer, CHUNK_SIZE*sizeof(game_results), MPI_BYTE, i+1, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_req[i]);
			// MPI_Wait(&recv_req[i], MPI_STATUS_IGNORE);
			if(!chunk_size_flag) {
				puzzle_copy(send_buf[idle_proc-1], CHUNK_SIZE, serv);
				MPI_Isend(&send_buf[idle_proc-1], IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, idle_proc, 1, MPI_COMM_WORLD, &send_req[idle_proc-1]); // for Isend: &send_req[idle_proc-1]
				MPI_Irecv(&recv_buffer[idle_proc-1], CHUNK_SIZE*sizeof(game_results), MPI_BYTE, MPI_ANY_SOURCE, RET_RESULT_TAG, MPI_COMM_WORLD, &recv_req[idle_proc-1]);
				//MPI_Wait(&send_req[i], MPI_STATUS_IGNORE);
				num_puzzles -= CHUNK_SIZE;
				chunk_size_flag = (num_puzzles < CHUNK_SIZE);
				// cout << "MORE WORK DISTRIBUTED TO PROCESS " << idle_proc << endl;
				// cout << "puzzles remaining: " << num_puzzles << endl;
			}
			else {
				MPI_Send(&send_buf[idle_proc-1], IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, idle_proc, STOP_PROC_TAG, MPI_COMM_WORLD);
				finished_proc += 1;
			}
		}
		else if(num_puzzles > 0){
			// cout << "ENTERED STAGE OF SERVER WHERE IT DOES LOCAL WORK WHILE WAITING.";
			// cout << endl << "PUZZLES REMAINING BEFORE SOLVING: " << num_puzzles << endl;
			if(num_puzzles > 0) {
				if(num_puzzles < SERVER_CHUNK)
					SERVER_CHUNK = num_puzzles;
				solve_locally(serv, SERVER_CHUNK);
				num_puzzles -= SERVER_CHUNK;
			}
		}
	}
	// cout << "EXITING THE MAIN WHILE LOOP IN SERVER" << endl;
	// DELETE buf here
	record_output(serv, output);
}


// Put the code for the client here
void Client(int rank){
	// send buf[] to the client processor and use game_board.Init() on the Client to
	// initialize game board. The result that will need to be sent back will either be
	// the moves required to solve the game or an indication that the game was not solvable.
	// cout << "ENTERING CLIENT FUNCTION FOR PROCESS " << rank << endl;
	bool exit_flag = false;
	MPI_Request send_req;
	MPI_Request recv_req;
	MPI_Status recv_status;
	unsigned char buf[IDIM*JDIM*CHUNK_SIZE]; // receive buffer
	MPI_Recv(buf, IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_status);
	game_results results[CHUNK_SIZE];
	while(!exit_flag){
		// int recv_tag = recv_status.MPI_TAG;
		search(buf, results, CHUNK_SIZE, rank);
		MPI_Send(results, CHUNK_SIZE*sizeof(game_results), MPI_BYTE, 0, RET_RESULT_TAG, MPI_COMM_WORLD);
		// if MPI receive value is specific status, break
		//MPI_Wait(&send_req, MPI_STATUS_IGNORE);
		MPI_Recv(buf, IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_status);
		//MPI_Iprobe(0, STOP_PROC_TAG, MPI_COMM_WORLD, &exit_flag, MPI_STATUS_IGNORE);
		exit_flag = (recv_status.MPI_TAG == STOP_PROC_TAG);
	}
	// cout << "PROCESS " << rank << " EXITING NOW." << endl;
}

int main(int argc, char *argv[]) {
	// This is a utility routine that installs an alarm to kill off this
	// process if it runs to long.  This will prevent jobs from hanging
	// on the queue keeping others from getting their work done.
	get_timer();
	chopsigs_();
	// All MPI programs must call this function
	MPI_Init(&argc, &argv);
	int myId;
	int numProcessors;
	/* Get the number of processors and my processor identification */
	MPI_Comm_size(MPI_COMM_WORLD, &numProcessors);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);
	// cout << "TIME TAKEN IN main() PRIOR TO ENTERING SERVER AND CLIENT: " << get_timer() << endl;
	if (myId == 0) {// Processor 0 runs the server code
		get_timer(); // zero the timer
		Server(argc, argv, numProcessors);
		// Measure the running time of the server
		cout << "execution time = " << get_timer() << " seconds." << endl;
	}
	else {// all other processors run the client code.
		Client(myId);
	}
	get_timer();
	// All MPI programs must call this before exiting
	// cout << "HITTING MPI_Finalize" << endl;
	MPI_Finalize();
}
