After login to titan.hpc.msstate.edu, you will need to login to the head node
of the shadow cluster with the command:

ssh -Y shadow-login


Before you use the shadow-login system for the first time you will need
load in the mpi package which you can do with the commands

module load openmpi slurm
module save default

This will load the openmpi verion of the MPI libraries and the slurm batch 
processing system into your executable path.  The "module save" command will 
save this setting so that it will apply to subsequent logins.


You can use the editors vi, nano, or emacs to perform this editing. 

*   nano is the easiest editor to use for the uninitiated.

Now you will need to copy the project1.tar file to talon-login (you only need
to do this once).  To copy this you can use the command

Then you can copy the project files to shadow from titan using the command 
(assuming you are logged into shadow-login)

scp titan:/scratch/CSE4163/Project1/project1.tar .

This will copy the file from titan to your home directory.

To unpack this file just use the command:

tar xvf project1.tar


You will now have a directory called 'project1' that will contain the starting
source code for your project.  Enter the directory using the command 
'cd project1'


Then you can compile using the command "make" in the project directory.  This
assumes that you have performed the module load command discussed above.


This directory contains several program files:

game.h:      This file defines a structs that is used in solving a puzzle
game.cc:     Implementation of methods defined in game.h
utilities.h: Define utility routines that will measure time and kill runaway
             jobs.
utilities.cc:Implementation of utility routines
main.cc:     Program main and implementation of server and client code.

easy_sample.dat:  A sample of puzzles that are computationally easy to solve
                  (to be used for debugging program)
hard_sample.dat:  A sample of puzzles that are computationally hard to solve
                  (to be used for measuring program performance)

debug0?.js:  A selection of job scripts for debugging runs on the
             parallel cluster

run??.js:    A selection of job scripts for performance runs on the
             parallel cluster
-----------------------------------------------------------------------------

How to submit jobs to the parallel cluster using the Slurm batch system:

To submit a job once the program has been compiled use one of the
provided Slurm job scripts (these end in .js).  These job scripts have
been provided to run parallel jobs on the cluster.  To submit a job use 
the "sbatch" command. (note:  "man sbatch" to get detailed information on this 
command)

example:
sbatch debug01.js

To see the status of jobs on the queue, use squeue.  Example:

Shadow-login-1[134] lush$ squeue -u lush
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON) 
             20298   48p160h  Work32P     lush PD       0:00      2 (ReqNodeNotAvail, UnavailableNodes:Shadow-1-[01-40],Shadow-2-[01-40],Shadow-3-[01-40],Shadow-4-[01-40]) 
             20297   48p160h  Work16P     lush PD       0:00      1 (ReqNodeNotAvail, UnavailableNodes:Shadow-1-[01-40],Shadow-2-[01-40],Shadow-3-[01-40],Shadow-4-[01-40]) 

This lists information associated with the job.  The important things to note
are the Job id and the state (ST).  The state tells the status of the job.  
Generally the status will be one of two values:  PD -  for pending

Additionally, if you decide that you don't want to run a job, or it seems to
not work as expected, you can run "qdel Job id" to delete it from the queue.
For example, to remove the above job from the queue enter the command

scancel 20297
