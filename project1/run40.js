#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Work40P
#SBATCH --nodes=2
#SBATCH --ntasks=32
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=20
#SBATCH --time=00:20:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
mpirun -np 40 ./project1 hard_sample.dat sol_hard.40 >& results.40p
