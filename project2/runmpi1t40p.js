#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Fluid20t2p
#SBATCH --nodes=2
#SBATCH --ntasks=40
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=20
#SBATCH --time=00:30:00
#SBATCH --no-reque
#SBATCH --qos=debug
export OMP_NUM_THREADS=1
mpirun -np 40 ./fluidmpi -n 64 -o fkte1t40p.dat >& out.mpi1t40p



