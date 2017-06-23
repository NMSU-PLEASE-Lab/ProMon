#!/bin/bash
#PBS -l nodes=1:ppn=16:gpu-all
#PBS -l walltime=04:55:00
#PBS -j eo -N lulesh.run

. /usr/share/Modules/init/bash
module purge
module load gcc-4.7.2
module load mvapich2-2.0/intel

export COMM_TYPE=MSGQUEUE
#export COMM_TYPE=POSIX
#export COMM_TYPE=EPOLL
#export PROMONHOME=/home/oaaziz/ProMon
#export JOB_SYSTEM=PBS
#export PROMONIP_INT=11.128.0.31
#export PROMONIP=$MYIP
#export PROMONIP=10.1.1.254
#export PROMONPORT=41111

rm /nmsu/apps/Omar/Proxyapps/lulesh2.0.3/lulesh2.0.promon

source $HOME/ProMon/hpc_examples/bigdat_script/Test_all/Gadget2/MSGQUEUE/pmsetup.sh /nmsu/apps/Omar/Proxyapps/lulesh2.0.3/lulesh2.0 /home/oaaziz/ProMon/hpc_pminput/lulesh_mainonly.xml

START=$(date +%s.%N)
mpirun $PMEXEC 
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is Omar= $DIFF"

#source $HOME/run/Gadget_run/pmshutdown.sh
