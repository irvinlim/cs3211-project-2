#!/bin/sh

# Specify the program parameters here.

PROG=pool
NUM_PROCS=36
LOG_LEVEL=4
FOLDER=tembusu
VAR=horizon
VAL=5

###########

# Prepare output files and directories.

cd ~/cs3211-project-2
DATADIR="data/$FOLDER/$PROG/$VAR/$VAL"
mkdir -p "$DATADIR"

SPECFILE="$FOLDER/$VAR/$VAL.txt"
cp "$SPECFILE" "$DATADIR/spec.txt"
SPECFILE="$DATADIR/spec.txt"

echo "Running $PROG/$VAR/$VAL on $NUM_PROCS processes with the following specifications:"
cat "$SPECFILE"
echo
echo "==="
echo

LOGFILE="$DATADIR/$PROG.log"
OUTPUTFILE=""
REPORTFILE=""
FRAMESDIR=""

echo > "$LOGFILE"

###########

# Make the program.
make clean
make

# Run the job.

export LOG_LEVEL

echo "Starting job at `date '+%Y-%m-%d %H:%M:%S'`..." >> "$LOGFILE"
echo "==============================================" >> "$LOGFILE"
echo "" >> "$LOGFILE"

COMMAND="mpirun --oversubscribe -np $NUM_PROCS $PROG $SPECFILE $OUTPUTFILE $REPORTFILE $FRAMESDIR"
echo Executing command: "$COMMAND" >> "$LOGFILE"
echo "LOG_LEVEL=$LOG_LEVEL" >> "$LOGFILE"
echo >> "$LOGFILE"

$COMMAND >> "$LOGFILE" 2>&1

echo "" >> "$LOGFILE"
echo "==============================================" >> "$LOGFILE"
echo "Job finished at `date '+%Y-%m-%d %H:%M:%S'`." >> "$LOGFILE"
