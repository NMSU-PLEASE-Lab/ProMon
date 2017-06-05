#!/bin/bash

. /usr/share/Modules/init/bash
module purge
module load gcc-4.7.2
module load  mvapich2-2.0/intel
#module load  mvapich2-2.0/gcc-4.7.2

# Now run an actual MPI program, miniMD;
# Use path as below, don't use "./" 
# mpirun /nmsu/apps/Mantevo/miniMD/miniMD_ref/miniMD 

START=$(date +%s.%N)
mpirun -np 16 /nmsu/apps/Omar/Mantevo/miniMD/miniMD_ref/miniMD_openmpi -i /nmsu/apps/Omar/Mantevo/miniMD/miniMD_ref/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The thime is Omar= $DIFF"

# NOTE:
# You do not need to specify a host file or the number of processes,
# mpirun will pick all of that info up automatically from Torque. Your
# MPI program will execute on all the nodes and cores allocated, one
# MPI process per core/thread.
#

