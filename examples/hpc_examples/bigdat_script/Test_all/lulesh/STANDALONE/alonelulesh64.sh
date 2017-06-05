#!/bin/bash

#PBS -l nodes=4:ppn=16:fpga-all
#PBS -l walltime=04:35:00
#PBS -j eo -N lulesh.run
#PBS -n


# You can do anything in a job script that you can in a script; here I
# am just creating stdout lines that print some info; this executes
# only once, on the job starting node, it does not execute on all nodes
echo "MY job:"
echo "jobid=${PBS_JOBID} jobname=${PBS_JOBNAME}"
echo "num-nodes=${PBS_NUM_NODES} num-processes=${PBS_NP} ppn=${PBS_NUM_PPN}"
echo ""

# Load the same MPI modules that we compiled with, so that the
# proper runtime libraries and commands are available

export LD_LIBRARY_PATH=/nmsu/tools/dwarf-20140413/lib:/nmsu/tools/DyninstAPI-8.1.2/lib:/nmsu/tools/gsl-1.15/lib:/opt/intel/lib/intel64
export DYNINSTAPI_RT_LIB=/nmsu/tools/DyninstAPI-8.1.2/lib/libdyninstAPI_RT.so

. /usr/share/Modules/init/bash
module purge
module load gcc-4.7.2
module load mvapich2-2.0/intel
#module load mvapich2-2.0/gcc-4.7.2

# Now run an actual MPI program, Gadget2;
# Use path as below, don't use "./" 
# mpirun /nmsu/apps/Gadget-2.0.6/Gadget2/Gadget2 /nmsu/apps/Gadget-2.0.6/Gadget2/parameterfiles/galaxy.param 

#mpirun /nmsu/apps/Gadget-2.0.6/Gadget2/Gadget2 /nmsu/apps/Gadget-2.0.6/Gadget2/parameterfiles/galaxy.param

x=`cat $PBS_NODEFILE |awk '{gsub(".cluster", "");print}'| xargs -n1 | sort -u | xargs | sed 's/\>/,/g;s/,$//' | tr -d ' '`
echo $x

# For Papi LDMS purpose
echo 'Write appfile to nodes'
act_exec --nodes=$x "echo 'lulesh' > /tmp/appname" > /dev/null 2>&1
act_exec --nodes=$x "echo ${PBS_JOBID} >> /tmp/appname" > /dev/null 2>&1
act_exec --nodes=$x "echo 'oaaziz' >> /tmp/appname" > /dev/null 2>&1

sleep 5

# NOTE:
# You do not need to specify a host file or the number of processes,
# mpirun will pick all of that info up automatically from Torque. Your
# MPI program will execute on all the nodes and cores allocated, one
# MPI process per core/thread.


# ProMon needs a job ID from some environment variable
export PBS_JOBID=${PBS_JOBID}
#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#
export APPHOME=/nmsu/apps/Omar/Proxyapps/lulesh2.0.3
export XMLFILES=/home/oaaziz/ProMon
export COMM_TYPE=MSGQUEUE
export PROMONHOME=/home/oaaziz/ProMon

#mpirun -np 2 -x PBS_JOBID $PROMONHOME/promon_injector DynamicInst $XMLFILES/pminput/Gadget2.xml $APPHOME/Gadget2 $XMLFILES/pminput/galaxy/galaxy.param 

START=$(date +%s.%N)
mpirun $APPHOME/lulesh2.0 
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is Omar = $DIFF"

# For Papi LDMS purpose
act_exec --nodes=$x 'rm /tmp/appname' > /dev/null 2>&1
