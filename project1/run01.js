#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Work01P
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:20:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
mpirun -np 1 ./project1 hard_sample.dat sol_hard.01 >& results.01p
