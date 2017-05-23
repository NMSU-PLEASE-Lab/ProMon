#!/bin/sh
#
# Run the test injector on the test application 
#

# Library path needs to find Dyninst libs
export LD_LIBRARY_PATH=/home/jcook/tools/lib

# Dyninst needs to be able to find its RT library
export DYNINSTAPI_RT_LIB=/home/jcook/tools/lib/libdyninstAPI_RT.so

# args after testapp are args FOR testapp
./injector testapp hello world

