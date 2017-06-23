#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=4
#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#
export APPHOME=$HOME/HPC_APPS/hmmer-2.3.2-MPI-0.92

mpirun -np 4 -machinefile $PROMONHOME/test/machinefile -x PBS_JOBID $PROMONHOME/promon_injector $PROMONHOME/pminput/hmmsearch.xml $APPHOME/bin/hmmsearch --mpi $APPHOME/tutorial/hadi.hmm $APPHOME/tutorial/uniprot_sprot.fasta



