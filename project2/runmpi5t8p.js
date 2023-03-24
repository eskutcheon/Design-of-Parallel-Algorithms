#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Fluid20t2p
#SBATCH --nodes=2
#SBATCH --ntasks=8
#SBATCH --cpus-per-task=5
#SBATCH --ntasks-per-node=4
#SBATCH --time=00:30:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
srun ./fluidmpi -n 64 -o fkte5t8p.dat >& out.mpi5t8p



