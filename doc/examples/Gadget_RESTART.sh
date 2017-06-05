#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#

# Library path needs to find Dyninst libs
export LD_LIBRARY_PATH=/home/hadi/dyninst-8.0/lib

# Dyninst needs to be able to find its RT library
export DYNINSTAPI_RT_LIB=/home/hadi/dyninst-8.0/lib/libdyninstAPI_RT.so

# ProMon needs a job ID from some environment variable
export SLURM_JOB_ID=9

#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

mpirun -np 2 $PROMONHOME/promon_injector $PROMONHOME/pminput/Gadget2.xml /home/hadi/HPC_APPS/Gadget-2.0.7/Gadget2/Gadget2 /home/hadi/HPC_APPS/Gadget-2.0.7/Gadget2/parameterfiles/galaxy.param 2 
