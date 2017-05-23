#!/bin/bash
#PBS -l nodes=4:ppn=48
#PBS -l walltime=24:55:00
#PBS -j eo -N sciptall.run

export LD_LIBRARY_PATH=/nmsu/tools/dwarf-20140413/lib:/nmsu/tools/DyninstAPI-8.1.2/lib:/nmsu/tools/gsl-1.15/lib:/opt/intel/lib/intel64
export DYNINSTAPI_RT_LIB=/nmsu/tools/DyninstAPI-8.1.2/lib/libdyninstAPI_RT.so

. /usr/share/Modules/init/bash
module purge
module load gcc-4.7.2
module load mvapich2-2.0/intel


#Copy the old progress file
#cp /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress_easy1.txt


export APPHOME=/nmsu/apps/Gadget-2.0.6/Gadget2
export PROMONHOME=/home/oaaziz/ProMon
export JOB_SYSTEM=PBS
export PROMONIP=10.1.1.254
export PROMONPORT=41111


# Start LDMS
sleep 2 
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sleep 30
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/remove.sh
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh &
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh &
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh
sleep 30

#
#	MiniMD
#

ProgramName="miniMD"

#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>				 16 process 		 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


OutputFile=/nmsu/omar_data/easy_miniMD16LDMS.csv
PromonFile=/nmsu/apps/omar_miniMD/miniMD_openmpi.promon
NumOProc=16

echo "************************************************"
echo "** Start $ProgramName 16 with Promon-Stand-alone **"
echo "************************************************"

echo "Start $ProgramName - 16 with Promon-Stand-alone" > /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts1_16.txt /nmsu/apps/omar_miniMD/miniMD_openmpi -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Stand-alone time = $DIFF"
echo "Stop $ProgramName - 16 with Promon-Stand-alone Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

sleep 2


echo "*****************************************"
echo "** Start $ProgramName 16 with Promon-LDMS **"
echo "*****************************************"

> /home/oaaziz/ovis/csvstore/csv/node/nodes_agg_promoncol.csv

export COMM_TYPE=MSGQUEUE

rm $PromonFile

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_miniMD/miniMD_openmpi /home/oaaziz/ProMon/hpc_pminput/miniMD.xml ${PROMONIP}

echo "Start $ProgramName - 16 with Promon-LDMS" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts1_16.txt $PMEXEC -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Promon-LDMS time = $DIFF"
echo "Stop $ProgramName - 16 with Promon-LDMS Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

sleep 120 
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sleep 30
cp /home/oaaziz/ovis/csvstore/csv/node/nodes_agg_promoncol.csv /nmsu/omar_data/easy_miniMD16LDMS.csv
sleep 5
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/remove.sh
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh &
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh &
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh
sleep 30


echo "*********************************************"
echo "** Start $ProgramName 16 with Promon-Analyzer **"
echo "*********************************************"

export PROMONHOME=/home/oaaziz/ProMon
export JOB_SYSTEM=PBS
export PROMONPORT=41111
export COMM_TYPE=EPOLL

export PROMONIP=$(ifconfig | awk '/inet addr/{print substr($2,6)}' | awk '{print $1; exit}')
echo "The IP is = $PROMONIP"
sed "1s/.*/"PROMONIP=$PROMONIP"/" ${PROMONHOME}/ProMon.cfg1 > ${PROMONHOME}/ProMon.cfg
cd /home/oaaziz/ProMon
${PROMONHOME}/promon_analyzer &> pma.out.$PBS_JOBID &
#ssh bigdat 'bash -s' < /home/oaaziz/ProMon/promon_analyzer &> pma.out.$PBS_JOBID &
# allow server time to initialize
sleep 5
echo "Promon server running on $PROMONIP"

rm $PromonFile

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_miniMD/miniMD_openmpi /home/oaaziz/ProMon/hpc_pminput/miniMD.xml ${PROMONIP} ${PROMONIP}

echo "Start $ProgramName - 16 with Promon-Analyzer" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts1_16.txt $PMEXEC -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Promon-Analyzer time = $DIFF"
echo "Stop $ProgramName - 16 with Promon-Analyzer Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt


#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>				 32 process 		 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


OutputFile=/nmsu/omar_data/easy_miniMD32LDMS.csv
PromonFile=/nmsu/apps/omar_miniMD/miniMD_openmpi.promon
NumOProc=32

echo "************************************************"
echo "** Start $ProgramName 32 with Promon-Stand-alone **"
echo "************************************************"

echo "Start $ProgramName - 32 with Promon-Stand-alone" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts2_16.txt /nmsu/apps/omar_miniMD/miniMD_openmpi -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Stand-alone time = $DIFF"
echo "Stop $ProgramName - 32 with Promon-Stand-alone Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

sleep 2


echo "*****************************************"
echo "** Start $ProgramName 32 with Promon-LDMS **"
echo "*****************************************"

> /home/oaaziz/ovis/csvstore/csv/node/nodes_agg_promoncol.csv

export COMM_TYPE=MSGQUEUE

rm $PromonFile

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_miniMD/miniMD_openmpi /home/oaaziz/ProMon/hpc_pminput/miniMD.xml ${PROMONIP}

echo "Start $ProgramName - 32 with Promon-LDMS" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts2_16.txt $PMEXEC -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Promon-LDMS time = $DIFF"
echo "Stop $ProgramName - 32 with Promon-LDMS Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

sleep 120 
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sleep 30
cp /home/oaaziz/ovis/csvstore/csv/node/nodes_agg_promoncol.csv /nmsu/omar_data/easy_miniMD32LDMS.csv
sleep 5
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/remove.sh
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh &
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh &
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh
sleep 30


echo "*********************************************"
echo "** Start $ProgramName 32 with Promon-Analyzer **"
echo "*********************************************"

export PROMONHOME=/home/oaaziz/ProMon
export JOB_SYSTEM=PBS
export PROMONPORT=41111
export COMM_TYPE=EPOLL

export PROMONIP=$(ifconfig | awk '/inet addr/{print substr($2,6)}' | awk '{print $1; exit}')
echo "The IP is = $PROMONIP"
sed "1s/.*/"PROMONIP=$PROMONIP"/" ${PROMONHOME}/ProMon.cfg1 > ${PROMONHOME}/ProMon.cfg
cd /home/oaaziz/ProMon
${PROMONHOME}/promon_analyzer &> pma.out.$PBS_JOBID &
#ssh bigdat 'bash -s' < /home/oaaziz/ProMon/promon_analyzer &> pma.out.$PBS_JOBID &
# allow server time to initialize
sleep 5
echo "Promon server running on $PROMONIP"

rm $PromonFile

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_miniMD/miniMD_openmpi /home/oaaziz/ProMon/hpc_pminput/miniMD.xml ${PROMONIP} ${PROMONIP}

echo "Start $ProgramName - 32 with Promon-Analyzer" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts2_16.txt $PMEXEC -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Promon-Analyzer time = $DIFF"
echo "Stop $ProgramName - 32 with Promon-Analyzer Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt


#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>				 64 process 		 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


OutputFile=/nmsu/omar_data/easy_miniMD64LDMS.csv
PromonFile=/nmsu/apps/omar_miniMD/miniMD_openmpi.promon
NumOProc=64

echo "************************************************"
echo "** Start $ProgramName 64 with Promon-Stand-alone **"
echo "************************************************"

echo "Start $ProgramName - 64 with Promon-Stand-alone" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts4_16.txt /nmsu/apps/omar_miniMD/miniMD_openmpi -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Stand-alone time = $DIFF"
echo "Stop $ProgramName - 64 with Promon-Stand-alone Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

sleep 2


echo "*****************************************"
echo "** Start $ProgramName 64 with Promon-LDMS **"
echo "*****************************************"

> /home/oaaziz/ovis/csvstore/csv/node/nodes_agg_promoncol.csv

export COMM_TYPE=MSGQUEUE

rm $PromonFile

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_miniMD/miniMD_openmpi /home/oaaziz/ProMon/hpc_pminput/miniMD.xml ${PROMONIP}

echo "Start $ProgramName - 64 with Promon-LDMS" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts4_16.txt $PMEXEC -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Promon-LDMS time = $DIFF"
echo "Stop $ProgramName - 64 with Promon-LDMS Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

sleep 120 
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sleep 30
cp /home/oaaziz/ovis/csvstore/csv/node/nodes_agg_promoncol.csv /nmsu/omar_data/easy_miniMD64LDMS.csv
sleep 5
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/remove.sh
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh &
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh &
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh
sleep 30


echo "*********************************************"
echo "** Start $ProgramName 64 with Promon-Analyzer **"
echo "*********************************************"

export PROMONHOME=/home/oaaziz/ProMon
export JOB_SYSTEM=PBS
export PROMONPORT=41111
export COMM_TYPE=EPOLL

export PROMONIP=$(ifconfig | awk '/inet addr/{print substr($2,6)}' | awk '{print $1; exit}')
echo "The IP is = $PROMONIP"
sed "1s/.*/"PROMONIP=$PROMONIP"/" ${PROMONHOME}/ProMon.cfg1 > ${PROMONHOME}/ProMon.cfg
cd /home/oaaziz/ProMon
${PROMONHOME}/promon_analyzer &> pma.out.$PBS_JOBID &
#ssh bigdat 'bash -s' < /home/oaaziz/ProMon/promon_analyzer &> pma.out.$PBS_JOBID &
# allow server time to initialize
sleep 5
echo "Promon server running on $PROMONIP"

rm $PromonFile

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_miniMD/miniMD_openmpi /home/oaaziz/ProMon/hpc_pminput/miniMD.xml ${PROMONIP} ${PROMONIP}

echo "Start $ProgramName - 64 with Promon-Analyzer" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts4_16.txt $PMEXEC -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Promon-Analyzer time = $DIFF"
echo "Stop $ProgramName - 64 with Promon-Analyzer Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>				 128 process 		 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


OutputFile=/nmsu/omar_data/easy_miniMD128LDMS.csv
PromonFile=/nmsu/apps/omar_miniMD/miniMD_openmpi.promon
NumOProc=128

echo "************************************************"
echo "** Start $ProgramName 128 with Promon-Stand-alone **"
echo "************************************************"

echo "Start $ProgramName - 128 with Promon-Stand-alone" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts4_32.txt /nmsu/apps/omar_miniMD/miniMD_openmpi -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Stand-alone time = $DIFF"
echo "Stop $ProgramName - 128 with Promon-Stand-alone Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

sleep 2


echo "*****************************************"
echo "** Start $ProgramName 128 with Promon-LDMS **"
echo "*****************************************"

> /home/oaaziz/ovis/csvstore/csv/node/nodes_agg_promoncol.csv

export COMM_TYPE=MSGQUEUE

rm $PromonFile

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_miniMD/miniMD_openmpi /home/oaaziz/ProMon/hpc_pminput/miniMD.xml ${PROMONIP}

echo "Start $ProgramName - 128 with Promon-LDMS" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts4_32.txt $PMEXEC -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Promon-LDMS time = $DIFF"
echo "Stop $ProgramName - 128 with Promon-LDMS Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt

sleep 120 
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/killit.sh
sleep 30
cp /home/oaaziz/ovis/csvstore/csv/node/nodes_agg_promoncol.csv /nmsu/omar_data/easy_miniMD128LDMS.csv
sleep 5
sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/remove.sh
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh &
sleep 30
ssh bigdat 'bash -s' < /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh &
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/test.sh
#sh /home/oaaziz/Res_svn_cook/omar/ovispublic_RC/ovis_test/MQSYSTEM5/start_agg.sh
sleep 30

echo "*********************************************"
echo "** Start $ProgramName 128 with Promon-Analyzer **"
echo "*********************************************"

export PROMONHOME=/home/oaaziz/ProMon
export JOB_SYSTEM=PBS
export PROMONPORT=41111
export COMM_TYPE=EPOLL

export PROMONIP=$(ifconfig | awk '/inet addr/{print substr($2,6)}' | awk '{print $1; exit}')
echo "The IP is = $PROMONIP"
sed "1s/.*/"PROMONIP=$PROMONIP"/" ${PROMONHOME}/ProMon.cfg1 > ${PROMONHOME}/ProMon.cfg
cd /home/oaaziz/ProMon
${PROMONHOME}/promon_analyzer &> pma.out.$PBS_JOBID &
#ssh bigdat 'bash -s' < /home/oaaziz/ProMon/promon_analyzer &> pma.out.$PBS_JOBID &
# allow server time to initialize
sleep 5
echo "Promon server running on $PROMONIP"

rm $PromonFile

source $HOME/ProMon/hpc_examples/bigdat_script/pmsetup.sh /nmsu/apps/omar_miniMD/miniMD_openmpi /home/oaaziz/ProMon/hpc_pminput/miniMD.xml ${PROMONIP} ${PROMONIP}

echo "Start $ProgramName - 128 with Promon-Analyzer" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt
START=$(date +%s.%N)
mpirun -np $NumOProc -hostfile /home/oaaziz/ProMon/hostfiles/hosts4_32.txt $PMEXEC -i /nmsu/apps/omar_miniMD/in.lj.miniMD
END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo "Promon-Analyzer time = $DIFF"
echo "Stop $ProgramName - 128 with Promon-Analyzer Time is $DIFF" >> /home/oaaziz/ProMon/hpc_examples/bigdat_script/Test_all/script_all/progress.txt


