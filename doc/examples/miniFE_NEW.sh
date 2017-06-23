#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=1

#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

export APPHOME=$HOME/HPC_APPS/miniFE_ref_1.5

echo mpirun -np 3 -x PBS_JOBID $PROMONHOME/promon_injector DynamicInst /home/hadi/research/PhD/Experiment-ProMon/pminput/miniFE.x.xml /home/hadi/HPC_APPS/miniFE_ref_1.5/miniFE.x nx200  itrNum200

