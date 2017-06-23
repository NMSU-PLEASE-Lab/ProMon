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
export COMM_TYPE=EPOLL
export PROMONIP="128.123.210.18"
export PROMONPORT=41111
export JOB_SYSTEM="PBS"
export PBS_JOBID=9


mpirun -np 3 -machinefile $PROMONHOME/test/machinefile -x PBS_JOBID $PROMONHOME/promon_injector BinaryInst $PROMONHOME/pminput/testapp.xml $PROMONHOME/test/testapp $PROMONHOME/test/test.txt
mpirun -np 3 -machinefile $PROMONHOME/test/machinefile -x COMM_TYPE -x PBS_JOBID -x PROMONIP -x PROMONPORT  $PROMONHOME/test/testapp.promon $PROMONHOME/test/test.txt

