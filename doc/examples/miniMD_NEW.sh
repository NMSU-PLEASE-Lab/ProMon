#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=2

#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#
export APPHOME=$HOME/HPC_APPS/miniMD_ref_1.0
export XMLFILES=$HOME/research/PhD/Experiment-ProMon

mpirun -np 4 -x PBS_JOBID $PROMONHOME/promon_injector DynamicInst $XMLFILES/pminput/miniMD_openmpi.xml $APPHOME/miniMD_openmpi -n 30 -i $APPHOME/in.lj.miniMD


