#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Ring20P
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=20
#SBATCH --time=00:20:00
#SBATCH --no-reque
#SBATCH --qos=debug

module load openmpi
mpirun -np 20 ./ring >& ring.out
