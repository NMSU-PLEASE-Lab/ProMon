#!/bin/sh
#
# Run ProMon on a test application
# -- NOTE YOU MUST FIRST START THE SERVER
#
# Injector usage is: monitoring-xml-spec application-binary application-arguments
#

export PBS_JOBID=123

$PROMONHOME/promon_injector DynamicInst $PROMONHOME/pminput/testapp.xml $PROMONHOME/test/testapp $PROMONHOME/test/test.txt 2


