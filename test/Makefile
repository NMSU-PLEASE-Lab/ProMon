CFLAGS := $(COMPIL_FLAGS) -lrt $(DEBUG)

ifeq ($(TARGET), MPI)
  CC=$(TARGETMPICC)
  CFLAGS:= $(CFLAGS) -DMT_WITH_MPI
else
  CC=$(TARGETCC)
endif

all: testapp

testapp: testapp.c
	$(CC) $(CFLAGS) testapp.c -o testapp 
	echo 'InitCondFile       $(PROMONHOME)/test/test.txt' | cat - testInput > temp && mv temp test.txt
	attr -q -s JOBID -V 122 test.txt 
	attr -q -s SNAPSHOTNO -V 456 test.txt
	
	
clean:
	rm -f *~ *.o testapp
