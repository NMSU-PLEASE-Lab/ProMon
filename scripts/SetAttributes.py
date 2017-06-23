# Currently we are setting the file attributes by python script
# This trent could change.
# I tried to use attr in the C++ but I couldn't get it to work.
# More time need to be spent on this topic
# The format to run this script is:
# python SetAttribute fileName userName jobMS jobid snapshNo)

import sys
import subprocess

p = subprocess.Popen('attr -q -s JOBID -V '+sys.argv[4]+ " " +sys.argv[1], shell=True)
p = subprocess.Popen('attr -q -s USERNAME -V '+sys.argv[2]+ " " +sys.argv[1], shell=True)
p = subprocess.Popen('attr -q -s JOBMGSYSTEM -V '+sys.argv[3]+ " " +sys.argv[1], shell=True)
p = subprocess.Popen('attr -q -s SNAPSHOTNO -V '+sys.argv[5]+ " " +sys.argv[1], shell=True)
