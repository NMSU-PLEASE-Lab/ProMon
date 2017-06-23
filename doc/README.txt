***Welcome to ProMon (ProMonitoring)***

This document, explains how to install ProMon and test it in the serial and MPI mode. 

============================================================
SETUP PROMON:
Prerequisites to ProMon installation: Dyninst (version 8.0 was tested), libdwarf (version 0.8.13 was tested), libdwarf (version 20121130 was tested) and Python (version 2.7.4 was tested) ***** CHECK THE setupTool_revised6182014.txt document to install Promon prerequisites *****

Installation of ProMon constitutes of two steps:
1- Compilation, 2- Configuration

COMPILATION:
ProMon can be compiled in serial and MPI mode. The mode can be defined in the Makefile in the ProMon directory. For MPI version set "TARGET = mpi" and for serial set "TARGET = serial". This setting is done on top of the Makefile.

Please change the MakeFile paths to:
  CXX = /home/hsharifi/APPS/gcc-4.8.1/bin/g++
  CC = /home/hsharifi/APPS/gcc-4.8.1/bin/gcc
  MPICC = /home/hsharifi/APPS/openmpi-1.6.5/bin/mpicc
  MPICXX = /home/hsharifi/APPS/openmpi-1.6.5/bin/mpic++
  DYNINSTPATH = /home/hsharifi/APPS/DyninstAPI-8.1.2
  BOOSTPATH = /home/hsharifi/APPS/boost_1_54_0

** in my case was: (because I have g++, gcc, mpic++, and mpicc all version 4.8.2 in my system)
  CXX = g++
  CC = gcc
  MPICC = mpicc
  MPICXX = mpic++
  DYNINSTPATH = /home/omar/APPS/DyninstAPI-8.1.2
  BOOSTPATH = /home/omar/APPS/boost_1_54_0

** export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/oaaziz/Build/libelf/lib:/home/oaaziz/Build/dwarf-20161124/lib

To compile ProMon project: make
To clean: make clean

Optional: make install (will create all library files in lib directory under ProMon)
Optional: make instpath=PATH install (will all library files in directory defined by instpath) 
Optional: make uninstall (will uninstall libraries from the lib directory under ProMon)
Optional: make instpath=PATH uninstall (will uninstall libraries from the PATH directory)

By compiling ProMon, three applications are created: promon_analyzer, promon_injector and promon_report.
promon_analyzer (analyzer): receives all the monitoring records and analyzes them.
promon_injector (injector): injects the monitoring instrumentation code to the running application. 
promon_report: provides a summary of generated information from the analyzer.

CONFIGURATION:
Set the PROMONHOME (where ProMon will be installed), DYNINSTPATH (where Dyninst is installed and contains include and bin directories) and DYNINSTAPI_RT_LIB (where it has be defined for Dyninst. Refere Dyninst documentation) in ~/.bash_profile or ~/.profile or /etc/profile file to the directory where ProMon source code is. 

Define the IP address, port number of the analyzer and the name of the job management system in the PromMon.cfg configuration file which is situated on top of ProMon directory. Here is an example of ProMon configuration: 

PROMONIP=127.0.0.1
PROMONPORT=41111
JOB_SYSTEM=SLURM

** export COMM_TYPE=EPOLL 

** export DYNINSTAPI_RT_LIB=$DYNINSTPATH/lib/libdyninstAPI_RT.so
EX. export DYNINSTAPI_RT_LIB=/nmsu/tools/DyninstAPI-8.1.2/lib/libdyninstAPI_RT.so

** export LD_LIBRARY_PATH=/nmsu/tools/dwarf-20140413/lib:/nmsu/tools/DyninstAPI-8.1.2/lib

 - to check the if your linker (ld) is linking the files test it by (ldd /nmsu/tools/DyninstAPI-8.1.2/lib/libdyninstAPI.so)


PROMONIP is the IP address for the promon_analyzer, PROMONPORT is the port number and JOB_SYSTEM is the job management system running the HPC application. These configurations are case sensitive and as for job management system, currently we support SLURM and PBS.

============================================================


**** TEST ***** 
Go to the main directory of ProMon and run (make testcase) to create the test folder
>> in the test folder vim mpirun.sh and change the mpirun to :
>> mpirun -np 2 $PROMONHOME/promon_injector DynamicInst $PROMONHOME/pminput/TestApp.xml $PROMONHOME/test/testapp $PROMONHOME/test/test.txt
**** TEST ***** 


TESTING PROMON IN SERIAL MODE:
****************************************:-
In order for ProMon to monitor an application using instrumentation technique, it needs an instruction on how to instrument the application and how to retrieve information related to the current run of the application.

The instruction for instrumentation is provided in the form of XML file with specific format. An sample of XML input for ProMon to instrument the simple sample code (serial or mpi) is in the "pminput" directory. The file is called "TestApp.xml".

The instruction on how to retrieve the information (new run, restart, restart from which job id, snapshot directory, snapshot file name format and snapshot number) related to running application is done by python scripts. A sample python code for the test application is in the "pminput". The file is called TestApp.py.

TEST PROMON IN SERIAL MODE:
Make sure that ProMon is compiled in serial mode. Test application is in the "test" directory. 
	1- In a terminal, run "./promon_analyzer"

to run the test application as a new job, 
	2- In a terminal, run "./test/serialrun.sh"

or, to run the test application as a restarted job, 
	2- In a terminal, run "./test/serialrun-restart.sh"

ProMon provides details of how time is spent in the application for each running processor. Currently, all running nodes will provide information but in future, a selected nodes (based on some numerical pattern) will send application's monitoring information. 
To see the summary of the running the application:

	./promon_report JOB_ID

For TestApp, to see the summary run the command as below:

	./promon_report 123
	
TESTING PROMON IN MPI MODE
To compile ProMon in MPI mode, set "TARGET = mpi" in the Makefile and run "make" command.
	1- In a terminal, run "./promon_analyzer"

to run the test application as a new job, 
	2- In a terminal, run "./test/mpirun.sh"

or, to run the test application as a restarted job, 
	2- In a terminal, run "./test/mpirun-restart.sh"
	
To see the summary of the running the application is same as serial.

============================================================
TESTING PROMON BY RUNNING GADGET2:

Refer to setupTool.txt for how to install Gadget2. The sample script that runs Gadget2 under SLURM is in /example directory. The script to run Gadget2 as a new run is called "newRun" and the script to run Gadget in restart mode from a snapshot is 
called "restartRun". A sample of XML input for ProMon to instrument Gadget2 is in the "pminput" directory. The file is called "gadget.xml". Based on that input data, the log files for Gadget is situated in directory GadgetIO/galaxy.
To run the test: 

	1- In a terminal, run "./promon_analyzer"
	2- In a terminal, run "./sbatch newRun"

To view summary of time spent in the running application:
	./promon_report JOBID

JOBID is an integer number for job id.
You can terminate the newRun to test the rework. In case the case of the running Gadget application from snapshot, the snapshot has to be copied with all the attributes. For this reason, the copy command in Linux should have "--preserve=xattr" parameters:

	cp --preserve=xattr  snapshot_001 galaxy_littleendian.dat

ProMon is using advanced file attributes which needs to have --preserve=xattr parameter to copy all the file attributes to the destination file.
	
To restart a run:
	sbatch restartRun
	
You can check the summary by 
	./promon_report JOBID



	


