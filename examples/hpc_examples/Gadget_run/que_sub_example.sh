#!/bin/bash
#PBS -l nodes=2:ppn=16:fpga-all
#PBS -l walltime=00:15:00
#PBS -j eo -N Gadget2.run

. /usr/share/Modules/init/bash
module purge
module load gcc-4.7.2
module load mvapich2-2.0/intel

export COMM_TYPE=MSGQUEUE

source $HOME/run/Gadget_run/pmsetup.sh /nmsu/apps/Gadget-2.0.6/Gadget2/Gadget2 /home/oaaziz/ProMon/hpc_pminput/Gadget2.xml

mpirun $PMEXEC /nmsu/apps/Gadget-2.0.6/Gadget2/parameterfiles/galaxy.param

#source $HOME/run/Gadget_run/pmshutdown.sh
