#!/bin/sh

regions=( 1 4 9 16 25 36 49 64 )
progs=( "pool" "poolseq" )

for region in "${regions[@]}"
do
    for prog in "${progs[@]}"
    do
        python ../log_parse_sum.py ../../data/tembusu/$prog/speedup_regions/$region/$prog.log
    done
done
