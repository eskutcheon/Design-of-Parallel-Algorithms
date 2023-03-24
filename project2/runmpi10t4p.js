#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Fluid10t4p
#SBATCH --nodes=2
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=10
#SBATCH --ntasks-per-node=2
#SBATCH --time=00:30:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
srun ./fluidmpi -n 64 -o fkte10t4p.dat >& out.mpi10t4p



