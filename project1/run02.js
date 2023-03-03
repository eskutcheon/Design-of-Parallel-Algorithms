#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Work02P
#SBATCH --nodes=1
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=2
#SBATCH --time=00:20:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
mpirun -np 2 ./project1 hard_sample.dat sol_hard.02 >& results.02p
