#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Debug02P
#SBATCH --nodes=1
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=2
#SBATCH --time=00:02:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
mpirun -np 2 ./project1 easy_sample.dat sol_easy.02 >& out.02p


