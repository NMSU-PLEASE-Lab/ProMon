#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
# PROMONHOME is defined as a prerequisite to perform make. It has to exist
#

# We give job id 123 to this test application.

export PBS_JOBID=123

mpirun -np 3 -machinefile $PROMONHOME/test/machinefile -x PBS_JOBID $PROMONHOME/promon_injector DynamicInst $PROMONHOME/pminput/testapp.xml $PROMONHOME/test/testapp $PROMONHOME/test/test.txt


