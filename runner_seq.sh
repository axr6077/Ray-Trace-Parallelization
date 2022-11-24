#!/bin/bash
#

# This is an example bash script that is used to submit a job
# to the cluster.
#
# Typcially, the # represents a comment.However, #SBATCH is
# interpreted by SLURM to give an option from above. As you 
# will see in the following lines, it is very useful to provide
# that information here, rather than the command line.

# Name of the job - You MUST use a unique name for the job
#SBATCH -J rt_seq

# Standard out and Standard Error output files
#SBATCH -o rt_seq_%j.out
#SBATCH -e rt_seq_%j.err

# In order for this to send emails, you will need to remove the
# space between # and SBATCH for the following 2 commands.
# Specify the recipient of the email
# SBATCH --mail-user=abc1234@rit.edu

# Notify on state change: BEGIN, END, FAIL or ALL
# SBATCH --mail-type=ALL

# Multiple options can be used on the same line as shown below.
# Here, we set the partition, number of cores to use, and the
# number of nodes to spread the jobs over.
# Since this is a sequential(one process) job, we only need one
# core and one node.
#SBATCH -p kgcoe-mps -n 1 -N 1
#SBATCH --mem-per-cpu=2000M

#
# Your job script goes below this line.
#

# Place your mpirun command here
# Notice that you have to provide the number of processes that
# are needed. This number needs to match the number of cores
# indicated by the -n option. If these do not, your results will
# not be valid or you may have wasted resources that others could
# have used.
./raytrace_seq -h 100 -w 100 -c configs/twhitted.xml -p none
