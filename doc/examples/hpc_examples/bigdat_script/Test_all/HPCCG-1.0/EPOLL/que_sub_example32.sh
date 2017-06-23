#!/bin/bash
#PBS -l nodes=2:ppn=16:gpu-all
#PBS -l walltime=04:35:00
#PBS -j eo -N HPCCG.run


. /usr/share/Modules/init/bash
module purge
module load gcc-4.7.2
module load mvapich2-2.0/intel

#export COMM_TYPE=MSGQUEUE
#export COMM_TYPE=POSIX
export COMM_TYPE=EPOLL
export PROMONHOME=/home/oaaziz/ProMon
export JOB_SYSTEM=PBS
#export PROMONIP_INT=11.128.0.31
#export PROMONIP=$MYIP
export PROMONIP=10.1.1.254
export PROMONPORT=41111

rm /nmsu/apps/omar_HPCCG-1.0/mpi/test_HPCCG.promon

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_HPCCG-1.0/mpi/test_HPCCG /home/oaaziz/ProMon/hpc_pminput/HPCCG_mainonly.xml

START=$(date +%s.%N)
mpirun $PMEXEC 128 128 1024
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The thime is Omar= $DIFF"

#source $HOME/run/Gadget_run/pmshutdown.sh
