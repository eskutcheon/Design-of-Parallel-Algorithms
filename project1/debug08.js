#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Debug08P
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=8
#SBATCH --time=00:02:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
mpirun -np 8 ./project1 easy_sample.dat sol_easy.08 >& out.08p
