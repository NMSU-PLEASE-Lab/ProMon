#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# make sure HOSTNAME is exported as some distros no longer export it by default
if test "$HOSTNAME" = ""; then
  export HOSTNAME=`hostname`
fi
# ProMon needs a job ID from some environment variable
export PBS_JOBID=7

#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

export APPHOME=$HOME/HPC_APPS/omnetpp-4.3/samples/cqn

mpirun -n 3 -x PBS_JOBID -x HOSTNAME $PROMONHOME/promon_injector DynamicInst $PROMONHOME/pminput/cqn.xml $APPHOME/cqn -n $APPHOME -c LargeLookahead -u Cmdenv 

###--parsim-communications-class=cMPICommunications

