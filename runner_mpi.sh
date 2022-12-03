#!/bin/bash
#

# Name of the job - You MUST use a unique name for the job
#SBATCH -J rt_mpi
#SBATCH --time=00:02:00
# Standard out and Standard Error output files
#SBATCH -o rt_mpi_%j.out
#SBATCH -e rt_mpi_%j.err

# In order for this to send emails, you will need to remove the
# space between # and SBATCH for the following 2 commands.
# Specify the recipient of the email
#SBATCH --mail-user=ayushrout96@gmail.com

# Notify on state change: BEGIN, END, FAIL or ALL
#SBATCH --mail-type=ALL

# Here, we set the partition, number of cores to use, the number
# of nodes to spread the jobs over, and allowed memory per CPU.
#SBATCH -p kgcoe-mps -n 16
#SBATCH --mem-per-cpu=2000M


# If the job that you are submitting is not sequential,
# then you MUST provide this line...it tells the node(s)
# that you want to use this implementation of MPI. If you
# omit this line, your results will indicate failure.
#spack load --first openmpi

# Place your srun command here
# Notice that you have to provide the number of processes that
# are needed. This number needs to match the number of cores
# indicated by the -n option. If these do not, your results will
# not be valid or you may have wasted resources that others could
# have used.

# **********************************************************************
# MAKE SURE THAT YOU ONLY HAVE ONE OF THESE UNCOMMENTED AT A TIME!
# **********************************************************************
# Sequential
# srun -n $SLURM_NPROCS raytrace_mpi -h 5000 -w 5000 -c configs/box.xml -p none 
# Static Strips
# srun -n $SLURM_NPROCS raytrace_mpi -h 5000 -w 5000 -c configs/box.xml -p static_strips_vertical 
# Static Cycles
# srun -n $SLURM_NPROCS raytrace_mpi -h 5000 -w 5000 -c configs/box.xml -p static_cycles_horizontal -cs 650
# Static Blocks
# srun -n $SLURM_NPROCS raytrace_mpi -h 5000 -w 100 -c configs/box.xml -p static_blocks 
# Dynamic
srun -n $SLURM_NPROCS raytrace_mpi -h 5000 -w 5000 -c configs/box.xml -p dynamic -bh 100 -bw 100 
