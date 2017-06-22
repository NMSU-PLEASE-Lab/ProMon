CFLAGS := $(COMPIL_FLAGS) $(DEBUG)

ifeq ($(TARGET), MPI)
  CXX=mpicxx
  CFLAGS:= $(CFLAGS) -DMT_WITH_MPI
else
  CXX=g++
endif

all: testapp

testapp: testapp.cpp
	$(CXX) $(CFLAGS) testapp.cpp -o testapp

clean:
	rm -f *~ *.o testapp
