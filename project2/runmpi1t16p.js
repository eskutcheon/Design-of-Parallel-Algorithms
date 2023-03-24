#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Fluid1t16p
#SBATCH --nodes=1
#SBATCH --ntasks=16
#SBATCH --cpus-per-task=1
#SBATCH --ntasks-per-node=16
#SBATCH --time=00:30:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
srun ./fluidmpi -n 64 -o fkte1t16p.dat >& out.mpi1t16p



