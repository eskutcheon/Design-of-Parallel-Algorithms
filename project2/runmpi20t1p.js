#!/bin/bash
#SBATCH --account=193000-cf0003
#SBATCH --job-name=Fluid10t2p
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=20
#SBATCH --ntasks-per-node=1
#SBATCH --time=00:30:00
#SBATCH --no-reque
#SBATCH --qos=debug
module load openmpi
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
./fluidomp -n 64 -o fkte20t1p.dat >& out.20t1p


