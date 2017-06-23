#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=8
#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

export APPHOME=$HOME/HPC_APPS/RAxML
export XMLFILES=$HOME/research/PhD/Experiment-ProMon

clean.sh
mpirun -n 2 -x SLURM_JOB_ID $PROMONHOME/promon_injector DynamicInst $PROMONHOME/pminput/raxmlHPC-MPI.xml $APPHOME/raxmlHPC-MPI -m GTRGAMMA -p 12345 -s $APPHOME/input/dna.phy -# 2000 -n hadi
