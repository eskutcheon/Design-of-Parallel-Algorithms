#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=FluidOMP40
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=40
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:30:00
#SBATCH --no-reque
#SBATCH --qos=debug
OMP_NUM_THREADS=40 ./fluidomp -n 64 -o fkte40.dat >& out.40thread



