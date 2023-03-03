#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Work16P
#SBATCH --nodes=1
#SBATCH --ntasks=16
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=16
#SBATCH --time=00:20:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
mpirun -np 16 ./project1 hard_sample.dat sol_hard.16 >& results.16p
