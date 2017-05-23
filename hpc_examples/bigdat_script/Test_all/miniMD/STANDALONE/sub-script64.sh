#!/bin/bash

#
# Use this file as: qsub sub-script.sh
#

#PBS -l nodes=4:ppn=16
#PBS -l walltime=04:35:00
#PBS -j eo -N miniMD.run
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
module load  mvapich2-2.0/intel
#module load  mvapich2-2.0/gcc-4.7.2

# Now run an actual MPI program, miniMD;
# Use path as below, don't use "./" 
# mpirun /nmsu/apps/Mantevo/miniMD/miniMD_ref/miniMD 

x=`cat $PBS_NODEFILE |awk '{gsub(".cluster", "");print}'| xargs -n1 | sort -u | xargs | sed 's/\>/,/g;s/,$//' | tr -d ' '`
echo $x

# For Papi LDMS purpose
echo 'Write appfile to nodes'
act_exec --nodes=$x "echo 'miniMD_openmpi' > /tmp/appname" > /dev/null 2>&1
act_exec --nodes=$x "echo ${PBS_JOBID} >> /tmp/appname" > /dev/null 2>&1
act_exec --nodes=$x "echo 'oaaziz' >> /tmp/appname" > /dev/null 2>&1

sleep 5

START=$(date +%s.%N)
mpirun /nmsu/apps/Omar/Mantevo/miniMD/miniMD_ref/miniMD_openmpi -i /nmsu/apps/Omar/Mantevo/miniMD/miniMD_ref/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The thime is Omar= $DIFF"

# NOTE:
# You do not need to specify a host file or the number of processes,
# mpirun will pick all of that info up automatically from Torque. Your
# MPI program will execute on all the nodes and cores allocated, one
# MPI process per core/thread.
#

