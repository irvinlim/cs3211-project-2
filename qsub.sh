#!/bin/sh

module load openmpi/intel/1.10.2

echo "Making project..."
make clean
make
echo 
echo "Submitting job..."
JOB_ID=`qsub submit.pbs`
echo 
echo "Fetching job stats for $JOB_ID..."
qstat "$JOB_ID"
