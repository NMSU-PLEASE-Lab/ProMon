#****************************************************************************
# Compile in serial or parallel (mpi) 

#TARGET = mpi
TARGET = serial

#
#****************************************************************************

# Place where dyninst is installed
DYNINSTPATH = /home/jcook/tools
BOOSTPATH = /home/jcook/tools

CC = gcc
CXX = g++
CXXFLAGS = -Wall -fPIC -I$(DYNINSTPATH)/include -I$(BOOSTPATH)

LFLAGS= -L$(DYNINSTPATH)/lib -ldyninstAPI -lrt -lpatchAPI \
		-lsymtabAPI -lpcontrol -linstructionAPI -lparseAPI \
		-ldyninstAPI_RT -ldynElf -ldynDwarf -lcommon -lstackwalk -lsymLite

#****************************************************************************

all: injector libprobe.so testapp

injector: injector.o
	$(CXX) -o injector injector.o $(LFLAGS) 

libprobe.so: probe.c
	$(CC) -shared -Wl,-soname,libprobe.so $^ -o $@

testapp: testapp.c

clean:
	rm  -f injector injector.o libprobe.so testapp *~ *.bak

