#
# ProMon setup script -- expected to be sourced near the beginning
# of a batch submission script, certainly before the application is
# run.
#
# Inputs:
# - requires $1 to be set to the application binary that will be run
# - requires $2 to be set to the XML monitoring specification to use
#
# Effects and Outputs:
# - will modify LD_LIBRARY_PATH
# - will generate an instrumented $EXEC.promon binary if it does not exist
#   or is out of date (needs to check spec file)
# - will set PMEXEC to point to the instrumented binary
# - will run the promon_analyzer server on the job launch node

# echo Args: $1 $2
PMEXEC=$1
PMSPEC=$2

#
# Setup ProMon stuff: it's home, the job system being used, and the
# IP/Port where the server will be running at
#
export PROMONHOME=/home/oaaziz/ProMon
export JOB_SYSTEM=PBS
#export PROMONIP_INT=11.128.0.31
export PROMONIP="10.1.1.21"
export PROMONPORT="41111"
# EPOLL, UDP, or LDMS
export COMM_TYPE=MSGQUEUE
#export COMM_TYPE=EPOLL
echo "The Comminication type is $COMM_TYPE \n"


# I needed to dynamically figure out the IP address for the server location, but
# you should be able to just use a static one as the commented out ones above
#export PROMONIP=`/sbin/ifconfig | grep '11.128' | awk -F: '{print $2}' | awk '{print $1}'`
echo "Promon server running on $PROMONIP"

#
# Run the event collector on the job start node
#
#
#cd /lscratch2/joncook/promon
cd /home/oaaziz/ProMon
# I think the server may not need the adjusted libpath, but it
# doesn't seem to bother anything right now; the ProMon tools
# are compiled using the Gnu compilers, and the miniGhost we are
# using is compiled with PGI, so our environment is not set up
# automatically for Gnu
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nmsu/tools/dwarf-20140413/lib:/nmsu/tools/DyninstAPI-8.1.2/lib:/nmsu/tools/gsl-1.15/lib:/opt/intel/lib/intel64
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nmsu/tools/dwarf-20140413/lib:/nmsu/tools/DyninstAPI-8.1.2/lib:/nmsu/tools/gsl-1.15/lib
#######################
# RUN PROMON AVALYZER #
#######################
#${PROMONHOME}/promon_analyzer &> pma.out.$PBS_JOBID &

# allow server time to initialize
sleep 2

#
# Now set up to run the instrumentation injector. The injector uses
# Dyninst but it only runs on the startup nodes -- it fails with an
# RT lib fault on compute nodes, so we cannot do "true" dynamic
# instrumentation. Instead we do pre-job instrumentation and right out
# a new binary that is instrumented
#
export LD_LIBRARY_PATH=${PROMONHOME}/probe:$LD_LIBRARY_PATH
export DYNINSTAPI_RT_LIB=/nmsu/tools/DyninstAPI-8.1.2/lib/libdyninstAPI_RT.so

INJECTOR=${PROMONHOME}/promon_injector
#PROMONSPEC=/home/joncook/pminput/miniGhost.xml

#
# We only create a new instrumented binary if it is necessary to.
# If it already exists we'll just use it.
# - should add check if the XML file is new, too
#
if [ ! -x $PMEXEC.promon -o $PMEXEC -nt $PMEXEC.promon ]; then
    echo  "The Command is: $INJECTOR BinaryInst $PMSPEC $PMEXEC"
    $INJECTOR BinaryInst $PMSPEC $PMEXEC
    echo "Instrumented $PMEXEC using $PMSPEC"
else
    echo "Instrumented binary for $PMEXEC is up to date. $PMSPEC $INJECTOR"
fi
PMEXEC=$PMEXEC.promon

echo "Use $PMEXEC as your application"
