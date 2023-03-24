#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Fluid10t2p
#SBATCH --nodes=1
#SBATCH --ntasks=2
#SBATCH --cpus-per-task=10
#SBATCH --ntasks-per-node=2
#SBATCH --time=00:30:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
srun ./fluidmpi -n 64 -o fkte10t2p.dat >& out.mpi10t2p

