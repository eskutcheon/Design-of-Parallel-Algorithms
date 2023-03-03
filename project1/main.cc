#include "game.h"
#include "utilities.h"
// Standard Includes for MPI, C and OS calls
#include <mpi.h>

// C++ standard I/O and library includes
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// C++ stadard library using statements
using std::cout ;
using std::cerr ;
using std::endl ;

using std::vector ;
using std::string ;

using std::ofstream ;
using std::ifstream ;
using std::ios ;

void Server(int argc, char *argv[]) {

  // Check to make sure the server can run
  if(argc != 3) {
    cerr << "two arguments please!" << endl ;
    MPI_Abort(MPI_COMM_WORLD,-1) ;
  }

  // Input case filename 
  ifstream input(argv[1],ios::in) ;

  // Output case filename
  ofstream output(argv[2],ios::out) ;

  int count = 0 ;
  int NUM_GAMES = 0 ;
  // get the number of games from the input file
  input >> NUM_GAMES ; // first line of the file
  for(int i=0;i<NUM_GAMES;++i) {  // for each game in file...
    string input_string ;
    input >> input_string ; // reads line by line
    // IDIM and JDIM defined as 5 in game.h, so they're global in the namespace
    if(input_string.size() != IDIM*JDIM) {
      cerr << "something wrong in input file format!" << endl ;
      MPI_Abort(MPI_COMM_WORLD,-1) ;
    }
    // read in the initial game state from file
    unsigned char buf[IDIM*JDIM] ; // new buffer for this iteration's game
    for(int j=0;j<IDIM*JDIM;++j)
      buf[j] = input_string[j] ;

    // Here we search for the solution to the game.   This is where most of
    // the work is performed.  We will want to farm these tasks out to other
    // processors in the parallel version.  To do this, send buf[] to the
    // client processor and use game_board.Init() on the Client to initialize
    // game board.  The result that will need to be sent back will either be
    // the moves required to solve the game or an indication that the game was
    // not solvable.

    // initialize the game
    game_state game_board ;
    game_board.Init(buf) ;

    // If we find a solution to the game, put the results in
    // solution
    move solution[IDIM*JDIM] ;
    int size = 0 ;
    // Search for a solution to the puzzle
    bool found = depthFirstSearch(game_board,size,solution) ;

    // If the solution is found we want to output how to solve the puzzle
    // in the results file.
    if(found) {
      output << "found solution = " << endl ;
      game_state s ;
      s.Init(buf) ;
      s.Print(output) ;
      for(int i=0;i<size;++i) {
        s.makeMove(solution[i]) ;
        output << "-->" << endl; 
        s.Print(output) ;
      }
      output << "solved" << endl ;
      count++ ;
    }
  } // end for
  // Report how cases had a solution.
  cout << "found " << count << " solutions" << endl ;

}

// Put the code for the client here
void Client() {
      // initialize the game
    game_state game_board ;
    game_board.Init(buf) ;
}


int main(int argc, char *argv[]) {
  // This is a utility routine that installs an alarm to kill off this
  // process if it runs to long.  This will prevent jobs from hanging
  // on the queue keeping others from getting their work done.
  chopsigs_() ;

  // All MPI programs must call this function
  MPI_Init(&argc,&argv) ;

  int myId ;
  int numProcessors ;

  /* Get the number of processors and my processor identification */
  MPI_Comm_size(MPI_COMM_WORLD,&numProcessors) ;
  MPI_Comm_rank(MPI_COMM_WORLD,&myId) ;

  if(myId == 0) {
    // Processor 0 runs the server code
    get_timer() ;// zero the timer
    Server(argc,argv) ;
    // Measure the running time of the server
    cout << "execution time = " << get_timer() << " seconds." << endl ;
  } else {
    // all other processors run the client code.
    Client() ;
  }

  // All MPI programs must call this before exiting
  MPI_Finalize() ;
}
