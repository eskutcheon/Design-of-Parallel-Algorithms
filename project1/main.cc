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

const int CHUNK_SIZE = 4;

struct game_results {
	game_state start_board;
	game_state game_board;
	move solution[IDIM*JDIM];
	bool found = false;
	int size = 0;
	int solved_by;
};

void search(unsigned char buf[][IDIM*JDIM], game_results results[], int &batch_size) {
	for (int i = 0; i < batch_size; ++i) {
		results[i].game_board.Init(buf[i]);
		results[i].start_board.Init(buf[i]);
		// Search for a solution to the puzzle
		results[i].found = depthFirstSearch(results[i].game_board,
											results[i].size,
											results[i].solution);
	}
}

void record_output(std::ostream &output, vector<game_results> &games) {
	int num_sol = games.size();
	for(int k = 0; k < num_sol) {
		output << "found solution = " << endl;
		games[k].start_board.Print(output);
		for (int i = 0; i < games[k].size; ++i) {
			games[k].start_board.makeMove(games[k].solution[i]);
			output << "-->" << endl;
			games[k].start_board.Print(output);
		}
		output << "solved" << endl;
	}
	cout << "found " << num_sol << " solutions" << endl;
}

// initial message passing can be done with regular MPI send and recv

void Server(int argc, char *argv[], int num_p) {
	// Check to make sure the server can run
	if (argc != 3) {
		cerr << "two arguments please!" << endl;
		MPI_Abort(MPI_COMM_WORLD, -1);
	}
	ifstream input(argv[1], ios::in);	// Input case filename
	ofstream output(argv[2], ios::out); // Output case filename
	vector<string> puzzles;				// will essentially act as the bag in the bag of tasks
	vector<game_results> solved_puzzles;
	int NUM_GAMES = 0;

	input >> NUM_GAMES; // get the number of games from first line of the input file
	// initialize a vector with all of the puzzles saved as a string
	for (int i = 0; i < NUM_GAMES; ++i) { // for each game in file...
		string input_string;
		input >> input_string; // reads line by line
		if (input_string.size() != IDIM*JDIM) {
			cerr << "something wrong in input file format!" << endl;
			MPI_Abort(MPI_COMM_WORLD, -1);
		}
		puzzles.push_back(input_string);
	}

	unsigned char buf[CHUNK_SIZE] = new unsigned char[IDIM*JDIM]; // new buffer for this iteration's game
	while (!puzzles.empty()) {
		// TODO: make the buffer a 2D array where each buffer is a chunk of game strings
		// CREATE BUFFER TO PASS ON
		for (int i = 0; i < CHUNK_SIZE; ++i) {
			// NOTE : not sure if myId is visible in this scope or not
			strcpy((char *)buf[i], puzzles[myId].c_str());
			puzzles.erase(puzzles.begin(), puzzles.begin()+numProcessors);
		}
		MPI_ISend(&buf[0], IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, myId, MPI_ANY_TAG, MPI_COMM_WORLD);
		// IDEA FOR LATER: Let the Server complete half of CHUNK_SIZE puzzles then check for idle processors
		// DEAL WITH SELECTION CORRECT BUFFER AND CHUNK_SIZE
		game_results results[CHUNK_SIZE];
		search(buf, results, CHUNK_SIZE);
		// MPI_Recv(results)
		for(int i=0; i < CHUNK_SIZE; ++i) {
			if (results[i].found) {
				solved_puzzles.push_back(results[i]);
			}
		}
	}
	// DELETE buf here
	record_output(output, solved_puzzles);
}

// Put the code for the client here
void Client(){
	// send buf[] to the client processor and use game_board.Init() on the Client to
	// initialize game board. The result that will need to be sent back will either be
	// the moves required to solve the game or an indication that the game was not solvable.
	// NOTE: missing final parameter MPI_Request *request
	while(true){
		MPI_Irecv(buf, IDIM*JDIM*CHUNK_SIZE, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD);
		// may want to dynamically allocate results array in case Client and Server keep too many in scope
		game_results results[CHUNK_SIZE];
		search(buf, results, CHUNK_SIZE);
		// MPI_ISend(results)
		// if MPI receive value is specific status, break
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
		Server(argc, argv, numProcessors);
		// Measure the running time of the server
		cout << "execution time = " << get_timer() << " seconds." << endl;
	}
	else {// all other processors run the client code.
		Client();
	}
	// All MPI programs must call this before exiting
	MPI_Finalize();
}
