#!/bin/bash
#PBS -l nodes=1:ppn=16:fpga-all
#PBS -l walltime=00:55:00
#PBS -j eo -N newtest.run

. /usr/share/Modules/init/bash
module purge
module load gcc-4.7.2
module load mvapich2-2.0/gcc-4.7.2


START=$(date +%s.%N)
mpirun -np 4 /home/oaaziz/ProMon/mpitest/a.out
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is = $DIFF"


START=$(date +%s.%N)
mpirun /home/oaaziz/ProMon/mpitest/a.out
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is = $DIFF"

START=$(date +%s.%N)
mpirun /home/oaaziz/ProMon/mpitest/a.out
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is = $DIFF"

START=$(date +%s.%N)
mpirun /home/oaaziz/ProMon/mpitest/a.out
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is = $DIFF"

START=$(date +%s.%N)
mpirun /home/oaaziz/ProMon/mpitest/a.out
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "The time is = $DIFF"

