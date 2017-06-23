#!/bin/bash


#PBS -l nodes=1:ppn=16:gpu-all
#PBS -l walltime=04:35:00
#PBS -j eo -N miniGhost.run

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
module load mvapich2-2.0/gcc-4.7.2

# Now run an actual MPI program, miniGhost;
# Use path as below, don't use "./" 
# mpirun /nmsu/apps/Mantevo/miniGhost/miniGhost_ref/miniGhost.x 
# Note:
#        - Execution is driven entirely by the default settings,
#          as configured in default-settings.h.
#        - Options may be listed using
#             ./miniGhost.x --help
#          or as seen in file MG_OPTIONS.F.
#        - Arguments can appear in any order.
#        - For more information see "/nmsu/apps/Mantevo/miniGhost/README" file,
#          Running miniGhost section.

START=$(date +%s.%N)
mpirun /nmsu/apps/omar_miniGhost/miniGhost.x 
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is Omar= $DIFF"


# NOTE:
# You do not need to specify a host file or the number of processes,
# mpirun will pick all of that info up automatically from Torque. Your
# MPI program will execute on all the nodes and cores allocated, one
# MPI process per core/thread.
#

