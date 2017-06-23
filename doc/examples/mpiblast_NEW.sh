#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=5

#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

export APPHOME=$HOME/HPC_APPS/mpiblast-1.6.0

mpirun -np 3 -machinefile $PROMONHOME/test/machinefile -x PBS_JOBID $PROMONHOME/promon_injector $PROMONHOME/pminput/mpiblast.xml $APPHOME/bin/mpiblast -d env_nt -i $APPHOME/storage/shared/hadi.fas -p blastn -o $APPHOME/storage/shared/results.txt
