#!/bin/bash

#
# Use this file as: qsub sub-script.sh
#

#PBS -l nodes=4:ppn=16
#PBS -l walltime=04:35:00
#PBS -j eo -N miniGhost.run
#PBS -n

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


x=`cat $PBS_NODEFILE |awk '{gsub(".cluster", "");print}'| xargs -n1 | sort -u | xargs | sed 's/\>/,/g;s/,$//' | tr -d ' '`
echo $x

# For Papi LDMS purpose
echo 'Write appfile to nodes'
act_exec --nodes=$x "echo 'miniGhost.x' > /tmp/appname" > /dev/null 2>&1
act_exec --nodes=$x "echo ${PBS_JOBID} >> /tmp/appname" > /dev/null 2>&1
act_exec --nodes=$x "echo 'oaaziz' >> /tmp/appname" > /dev/null 2>&1

sleep 5

SCALING=2
COMM_METHOD=10
COMM_METHOD_S=BSPMA
NX=100
NY=100
NZ=100
DIM_S=100x100x100
NUM_VARS=40
NUM_VARS_S=40vars
PERCENT_SUM=100
PERCENT_SUM_S=100summed
NUM_SPIKES=3
NUM_TSTEPS=80
STENCIL=21
STENCIL_S=2d5pt
ERROR_TOL=8
REPORT_DIFFUSION=10

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
mpirun /nmsu/apps/Omar/Mantevo/miniGhost/miniGhost_ref/miniGhost.x \
        --scaling $SCALING \
        --comm_method $COMM_METHOD \
        --nx $NX \
        --ny $NY \
        --nz $NZ \
        --num_vars $NUM_VARS \
        --percent_sum $PERCENT_SUM \
 #       --num_spikes $NUM_SPIKES \
  #      --num_tsteps $NUM_TSTEPS \
   #     --stencil $STENCIL \
    #    --error_tol $ERROR_TOL \
     #   --report_diffusion $REPORT_DIFFUSION \
#        --npx $NPX \
 #       --npy $NPY \
  #      --npz $NPZ



END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is Omar= $DIFF"


# NOTE:
# You do not need to specify a host file or the number of processes,
# mpirun will pick all of that info up automatically from Torque. Your
# MPI program will execute on all the nodes and cores allocated, one
# MPI process per core/thread.
#

