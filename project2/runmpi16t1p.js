#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Fluid16t1p
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:30:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
srun ./fluidmpi -n 64 -o fkte16t1p.dat >& out.mpi16t1p



