#!/bin/bash

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ujjwal/Source/dwarf-20161124/lib:/home/ujjwal/Build/DyninstAPI-9.3.2/lib:/home/ujjwal/Source/ProMon/probe
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ujjwal/Build/DyninstAPI-9.3.2/lib:/home/ujjwal/Source/ProMon2/probe:/home/ujjwal/Build/msgpack-2.1.3/lib
export DYNINSTAPI_RT_LIB=/home/ujjwal/Build/DyninstAPI-9.3.2/lib/libdyninstAPI_RT.so
export PROMONHOME=/home/ujjwal/Source/ProMon2
export LD_LIBRARY_PATH=${PROMONHOME}/probe:$LD_LIBRARY_PATH
export PROMONIP=127.0.0.1
export PROMONPORT=41111
export JOB_SYSTEM=PBS
# EPOLL, UDP, or LDMS
export COMM_TYPE=TCP

# This is for testing remove on actual job run.
export PBS_JOBID=123
