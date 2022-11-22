#!/bin/bash

#SBATCH -J raytrace_mpi
#SBATCH -o rt_mpi_%j.out
#SBATCH -e rt_mpi_%j.err

# In order for this to send emails, you will need to remove the
# space between # and SBATCH for the following 2 commands.
# Specify the recipient of the email
#SBATCH --mail-user=ayushrout96@rit.edu

# Notify on state change: BEGIN, END, FAIL or ALL
#SBATCH --mail-type=ALL

# Set the partition, number of cores to use, the number
# of nodes to spread the jobs over, and allowed memory per CPU.
# Here kgcoe-mps is the partition of the cluster 
#SBATCH -p kgcoe-mps -n 3
#SBATCH --mem-per-cpu=2000M

# Load the mpi framework into current session
#spack load --first openmpi

# **********************************************************************
# MAKE SURE THAT YOU ONLY HAVE ONE OF THESE UNCOMMENTED AT A TIME!
# **********************************************************************
# Sequential
#srun -n $SLURM_NPROCS raytrace_mpi -h 100 -w 100 -c configs/twhitted.xml -p none 
# Static Strips
# srun -n $SLURM_NPROCS raytrace_mpi -h 100 -w 100 -c configs/twhitted.xml -p static_strips_horizontal 
# Static Cycles
# srun -n $SLURM_NPROCS raytrace_mpi -h 5000 -w 5000 -c configs/box.xml -p static_cycles_horizontal -cs 650
# Static Blocks
# srun -n $SLURM_NPROCS raytrace_mpi -h 100 -w 100 -c configs/twhitted.xml -p static_blocks 
# Dynamic
srun -n $SLURM_NPROCS raytrace_mpi -h 5000 -w 5000 -c configs/twhitted.xml -p dynamic -bh 100 -bw 100 
