#!/bin/bash
export LD_LIBRARY_PATH=/nmsu/tools/dwarf-20140413/lib:/nmsu/tools/DyninstAPI-8.1.2/lib:/nmsu/tools/gsl-1.15/lib:/opt/intel/lib/intel64
export DYNINSTAPI_RT_LIB=/nmsu/tools/DyninstAPI-8.1.2/lib/libdyninstAPI_RT.so
sleep 2
# For Papi LDMS purpose
echo 'Write appfile to node'
echo 'Gadget' > /tmp/appname
sleep 2
echo 22222 >> /tmp/appname
sleep 2
echo 'oaaziz' >> /tmp/appname
sleep 2
mpirun -np 16 /nmsu/apps/Gadget-2.0.6/Gadget2/Gadget2 /nmsu/apps/Gadget-2.0.6/Gadget2/parameterfiles/OmarTest/cluster.param16
