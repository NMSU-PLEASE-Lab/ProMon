#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=6
#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

export APPHOME=$HOME/HPC_APPS/pop/build
export XMLFILES=$HOME/research/PhD/Experiment-ProMon

mpirun -np 5 -x SLURM_JOB_ID $PROMONHOME/promon_injector DynamicInst $XMLFILES/pminput/pop.xml $APPHOME/pop 
