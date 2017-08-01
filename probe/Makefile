CFLAGS := -Wall -O3 -fPIC  $(DEBUG_PROBFUNC)
ifeq ($(TARGET), MPI)
  COMPILER := $(TARGETMPICC)
  CFLAGS:= $(CFLAGS)
else
  COMPILER := $(TARGETCC)
endif

MAJOR := 1
MINOR := 0
VERSION := $(MAJOR).$(MINOR) 
LIBNAME := ProMon
OBJECTS = probe.o

all: lib
 
lib: lib$(LIBNAME).so.$(VERSION)
 
lib$(LIBNAME).so.$(VERSION): $(OBJECTS)
	$(COMPILER) -shared -Wl,-soname,lib$(LIBNAME).so.$(MAJOR) $^ -o $@
	ln -sf  lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so
	ln -sf  lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so.1 

probe.o: ../predefs.h probe.h probe.c
	$(COMPILER) $(CFLAGS) -c -o probe.o probe.c
	
#****************************************************************************

install:
ifeq ($(instpath),)
	cp lib$(LIBNAME).so.$(VERSION) ../lib;
	cd ../lib;ln -sf  lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so; ln -sf  lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so.1
else
	cp lib$(LIBNAME).so.$(VERSION) $(instpath);
	cd $(instpath);ln -sf  lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so; ln -sf  lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so.1
endif

#****************************************************************************

uninstall:
ifeq ($(instpath),)
	cd ../lib;rm lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so lib$(LIBNAME).so.1
else
	cd $(instpath);rm lib$(LIBNAME).so.$(VERSION) lib$(LIBNAME).so lib$(LIBNAME).so.1
endif

#****************************************************************************

clean:
	$(RM) *~ *.o *.so*

#****************************************************************************

test:
ifneq ($(instpath),)
	$(info instpath undefined)
	ls;
endif

