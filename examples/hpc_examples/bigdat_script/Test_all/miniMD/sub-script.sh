#!/bin/bash

#
# Use this file as: qsub sub-script.sh
#

# Request 4 nodes and 48 cores/node (total of 192 processes), allow
# Torque to allocate from "fpga-all" (all nodes with fpga on them), and limit 
# execution time to just 30 minutes

#PBS -l nodes=1:ppn=16:gpu-all
#PBS -l walltime=04:35:00
#PBS -j eo -N miniMD.run

# Many other PBS options could be set. See "man qsub", the "Script
# Processing" section. Also you can request different numbers of nodes
# and ppn.

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

# Now run an actual MPI program, miniMD;
# Use path as below, don't use "./" 
# mpirun /nmsu/apps/Mantevo/miniMD/miniMD_ref/miniMD 

mpirun /nmsu/apps/omar_miniMD/miniMD_openmpi -i /nmsu/apps/omar_miniMD/in.lj.miniMD

# NOTE:
# You do not need to specify a host file or the number of processes,
# mpirun will pick all of that info up automatically from Torque. Your
# MPI program will execute on all the nodes and cores allocated, one
# MPI process per core/thread.
#

