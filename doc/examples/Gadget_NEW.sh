#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# ProMon needs a job ID from some environment variable
export PBS_JOBID=9
#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#
export APPHOME=$HOME/APPS/Gadget-2.0.7/Gadget2
export XMLFILES=$HOME/ProMon
export COMM_TYPE=LDMS

#mpirun -np 2 -x PBS_JOBID $PROMONHOME/promon_injector DynamicInst $XMLFILES/pminput/Gadget2.xml $APPHOME/Gadget2 $XMLFILES/pminput/galaxy/galaxy.param 


mpirun -np 4 $PROMONHOME/promon_injector DynamicInst $XMLFILES/pminput/Gadget2.xml $APPHOME/Gadget2 $HOME/APPS/Gadget-2.0.7/galaxy/galaxy.param

