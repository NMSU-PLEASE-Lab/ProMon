#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=3

#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

export APPHOME=$HOME/HPC_APPS/gromacsbin/bin

mpirun -np 3 $PROMONHOME/promon_injector $PROMONHOME/pminput/mdrun.xml $APPHOME/mdrun -v -deffnm $PROMONHOME/experiments/gromacs/em -ntomp 1 -ntomp_pme 1 

###

