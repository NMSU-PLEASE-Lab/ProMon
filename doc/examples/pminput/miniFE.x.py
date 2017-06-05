# This script returns the details for new run in this format:
# SnapShotOutPutDir:SnapshotBaseName:NEWRUN
# for rstart run the format is:
# SnapShotOutPutDir:SnapshotBaseName:RESTART:RestartedJobID:RestartedFromSnapshotNo
# The format above is standard throught ProMon 
import sys

output = list()
output.insert(0, "/home/hadi/HPC_APPS/miniFE_ref_1.5")
output.insert(1, "XXX")
output.insert(2, "NEWRUN")

# We are not using regular print. 
# print function puts an extra '\n' which is not desirable
sys.stdout.write(':'.join(output))
