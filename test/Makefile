#Change these if necessary
DEBUG := -DDEBUG
TARGETCC := g++ -g
TARGETMPICC := mpic++

CFLAGS := $(COMPIL_FLAGS) -lrt $(DEBUG)

ifeq ($(TARGET), MPI)
  CC=$(TARGETMPICC)
  CFLAGS:= $(CFLAGS) -DMT_WITH_MPI
else
  CC=$(TARGETCC)
endif

all: testapp

testapp: testapp.cpp
	$(CC) $(CFLAGS) testapp.cpp -o testapp

clean:
	rm -f *~ *.o testapp
