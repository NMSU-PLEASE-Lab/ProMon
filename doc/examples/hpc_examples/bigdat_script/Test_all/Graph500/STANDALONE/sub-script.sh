#!/bin/bash

#
# Use this file as: qsub sub-script.sh
#

# Request 4 nodes and 16 cores/node (total of 64 processes), allow
# Torque to allocate from "fpga-all" (fpga cluster nodes), and limit 
# execution time to just 5 minutes

#PBS -l nodes=4:ppn=16:fpga-all
#PBS -l walltime=04:55:00
#PBS -j eo -N Graph500.run

# Many other PBS options could be set. See "man qsub", the "Script
# Processing" section. Also you can request different numbers of nodes
# and ppn (Total numer or processes must be power-of-2).

# You can do anything in a job script that you can in a script; here I
# am just creating stdout lines that print some info; this executes
# only once, on the job starting node, it does not execute on all nodes
echo "MY job:"
echo "jobid=${PBS_JOBID} jobname=${PBS_JOBNAME}"
echo "num-nodes=${PBS_NUM_NODES} num-processes=${PBS_NP} ppn=${PBS_NUM_PPN}"
echo ""

# Load the same MPI modules that we compiled with, so that the
# proper runtime libraries and commands are available

. /usr/share/Modules/init/bash
module purge
module load gcc-4.7.2
module load  mvapich2-2.0/intel

# Now run an actual MPI program, graph500 benchmark;
# Use path as below, don't use "./" 
# /nmsu/apps/graph500-2.1.4/g500-without-validation/mpi/graph500_mpi_simple n 
# where n is the SCALE " graph size = 2^SCALE".  

#mpirun /nmsu/apps/graph500-2.1.4/g500-original/mpi/graph500_mpi_simple 32

START=$(date +%s.%N)
mpirun /nmsu/apps/omar_g500-without-validation/mpi/graph500_mpi_simple 20
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The thime is Omar= $DIFF"

# NOTE:
# You do not need to specify a host file or the number of processes,
# mpirun will pick all of that info up automatically from Torque. Your
# MPI program will execute on all the nodes and cores allocated, one
# MPI process per core/thread.
#

