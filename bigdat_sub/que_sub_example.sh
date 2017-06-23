#!/bin/bash
#PBS -l nodes=1:ppn=16
#PBS -j oe -N miniGhost.xe6.testing
#PBS -l walltime=00:15:00
#PBS -j oe

source $HOME/bin/pmsetup.sh $HOME/apps/miniGhost_ref_1.0/miniGhost.x 
$HOME/pminp
ut/miniGhost.xml

mpirun $PMEXEC

source $HOME/bin/pmshutdown.sh
