#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=10

#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

export APPHOME=$HOME/HPC_APPS/clustalw-mpi-0.13
export XMLFILES=$HOME/research/PhD/Experiment-ProMon

mpirun -np 3 -x PBS_JOBID  $PROMONHOME/promon_injector DynamicInst $XMLFILES/pminput/clustalw-mpi.xml $APPHOME/clustalw-mpi -infile=$APPHOME/hadi.input

