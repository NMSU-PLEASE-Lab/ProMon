#****************************************************************************
# Customizin ProMon
#****************************************************************************
# ProMon needs PROMONHOME environment variable to compile and
# during runtime to read to create log files.
# PROMONHOME 

# Compile in serial or parallel (mpi) 
  #export TARGET = MPI
  export TARGET = serial

# The communication port can be selected among 
# TCP, UDP, EPOLL, and MSGQUEUE.
# The selection is done by setting COMM_TYPE.
# One easy way is to set the environment variable in Promon.cfg

# To compile in Debug mode, uncomment the line below.
  export DEBUG = -DDEBUG

# To compile monitoring probe functions in Debug mode, uncomment the line below.
  export DEBUG_PROBFUNC = -DDEBUG_PROBFUNC

# To enable logging uncomment 
# You can select default logging or Boost logging by next configuration
  export MT_WITH_LOG = -DMT_WITH_LOG

# ProMon uses Boost Log or Simple built in log
# To compile with Boost log component and disable the simple built in log
  #export BOOSTLOG = -DBOOSTLOG

# Customize this part based on your configuration.
#<<<<<<< .mine
# We need to have ./confiure app for Promon to define these variable.
  CXX = g++
  CC = gcc
  MPICC = mpicc
  MPICXX = mpicxx
  DYNINSTPATH = /home/ujjwal/Build/DyninstAPI-8.1.2
  BOOSTPATH = /home/ujjwal/Build/boost_1_54_0
#=======
# We need to have ./configure app for Promon to define these variable.
#  CXX = /home/hsharifi/APPS/gcc-4.8.1/bin/g++
#  CC = /home/hsharifi/APPS/gcc-4.8.1/bin/gcc
#  MPICC = /home/hsharifi/APPS/openmpi-1.6.5/bin/mpicc
#  MPICXX = /home/hsharifi/APPS/openmpi-1.6.5/bin/mpic++ 
#  DYNINSTPATH = /home/hsharifi/APPS/DyninstAPI-8.1.2
#  BOOSTPATH = /home/hsharifi/APPS/boost_1_54_0
#>>>>>>> .r889

# Compiler flags
  export COMPIL_FLAGS = -Wall -O0 -fPIC  

#****************************************************************************
# Do not change the make file from this part till end
#****************************************************************************

TINYXML = tinyxml
CPPFLAGS = $(COMPIL_FLAGS) $(DEBUG) $(MT_WITH_LOG) -I$(DYNINSTPATH)/include -I$(TINYXML) -I$(BOOSTPATH)

ifneq ($(BOOSTLOG),)
   CPPFLAGS := $(CPPFLAGS) -DBOOST_LOG_DYN_LINK -DBOOST_ALL_DYN_LINK -DMT_WITH_BOOST
   BOOSTLFLAGS = -I$(BOOSTPATH) -L$(BOOSTPATH)/stage/lib \
      -lboost_log -lboost_system -lboost_thread -lboost_filesystem \
      -lboost_date_time -lboost_chrono
endif

#
# We link tinyxml object files directly in rather than relying on
# a dynamic link library so that we don't have to worry about a runtime
# dependency.
#
EXTOBJS = $(TINYXML)/tinyxml.o $(TINYXML)/tinyxmlparser.o \
          $(TINYXML)/tinyxmlerror.o $(TINYXML)/tinystr.o

LDFLAGS= -L$(DYNINSTPATH)/lib -ldyninstAPI -lrt -lpatchAPI \
		-lsymtabAPI -lpcontrol -linstructionAPI -lparseAPI \
		-ldyninstAPI_RT -ldynElf -ldynDwarf -lcommon -lstackwalk -lsymLite 
LIBS= -lpthread -lrt

#******************************************************************************

all: check-env submakes promon_analyzer promon_report promon_injector

submakes:
	cd $(TINYXML); make lib TARGETCXX=$(CXX) TARGETCC=$(CC) 
	cd probe; make TARGETCXX=$(CXX) TARGETMPICXX=$(MPICXX) DYNINSTPATH=$(DYNINSTPATH) BOOSTPATH=$(BOOSTPATH) 

promon_injector: injector.o parser.o util.o
	$(CXX) -o promon_injector injector.o parser.o util.o $(EXTOBJS) $(LDFLAGS) $(BOOSTLFLAGS) 

promon_analyzer: analyzer.o serverEPoll.o serverUDP.o serverTCP.o serverMsgQueue.o util.o server.o 
	$(CXX) -o promon_analyzer analyzer.o serverEPoll.o serverUDP.o serverTCP.o serverMsgQueue.o util.o server.o $(LIBS) $(BOOSTLFLAGS)

promon_report: report.o
	$(CXX) -o promon_report report.o $(LIBS)

analyzer.o: predefs.h
report.o: predefs.h

#****************************************************************************

testcase:
	cd test; make TARGETCC=$(CC) TARGETMPICC=$(MPICC)  

#****************************************************************************

install:
ifeq ($(instpath),)
	$(info Installing in lib/)
	cd probe; make TARGET=$(TARGET) install;
	cd tinyxml; make install;
else
	$(info Installing in $(instpath))
	cd probe; make instpath=$(instpath) TARGET=$(TARGET) install;
	cd tinyxml; make instpath=$(instpath) install;
endif

#****************************************************************************

uninstall:
ifeq ($(instpath),)
	cd probe; make TARGET=$(TARGET) uninstall;
	cd tinyxml; make uninstall;
else
	cd probe; make instpath=$(instpath) TARGET=$(TARGET) uninstall;
	cd tinyxml; make instpath=$(instpath) uninstall;
endif

#****************************************************************************

clean:
	$(RM) *~ *.o promon_analyzer promon_report promon_injector;
	cd tinyxml; make clean
	cd probe; make TARGET=$(TARGET) clean
	cd test; make TARGET=$(TARGET) clean
	
#****************************************************************************

check-env:
ifndef PROMONHOME
	$(error PROMONHOME is undefined!!! Check doc/README.)
endif
ifndef DYNINSTPATH
        $(error DYNINSTPATH is undefined!!! Check doc/README.)
endif

