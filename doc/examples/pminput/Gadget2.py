# This script returns the details for new run in this format:
# SnapShotOutPutDir:SnapshotBaseName:NEWRUN
# for rstart run the format is:
# SnapShotOutPutDir:SnapshotBaseName:RESTART:RestartedJobID:RestartedFromSnapshotNo
# The format above is standard throught ProMon 
import sys
import os

input = str(sys.argv[1])
il = input.split(':')
output = list()

f = open(il[0])
for line in iter(f):
	l = line.split()
	if 'InitCondFile' in l:
		initFile = l[1]
	elif 'OutputDir' in l:
		output.insert(0, l[1])
	elif 'SnapshotFileBase' in l:
		output.insert(1, l[1])
f.close()

if len(il) == 2:
	output.insert(2, 'RESTART')
	p = os.popen('attr -q -g JOBID '+initFile,"r")
	output.insert(3, p.readline().strip(' ').strip())
	
	p = os.popen('attr -q -g SNAPSHOTNO '+initFile,"r")
        output.insert(4, p.readline().strip(' ').strip())
else :
	output.insert(2, 'NEW')

print ':'.join(output)

