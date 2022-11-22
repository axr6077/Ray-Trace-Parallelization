#!/bin/bash
#

# Name of the job - You MUST use a unique name for the job
#SBATCH -J raytrace_seq

# Standard out and Standard Error output files
#SBATCH -o rt_seq_%j.out
#SBATCH -e rt_seq_%j.err

# Specify the recipient of the email to send status
# SBATCH --mail-user=axr6077@rit.edu

# Notify on state change: BEGIN, END, FAIL or ALL
# SBATCH --mail-type=ALL

#SBATCH -p kgcoe-mps -n 1 -N 1
#SBATCH --mem-per-cpu=2000M

# 5000x5000 render with sequential algorithm (complex scene) 
./raytrace_seq -h 5000 -w 5000 -c configs/box.xml -p none
