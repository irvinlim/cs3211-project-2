#!/bin/sh

regions=( 500 1000 2000 4000 8000 16000 )
progs=( "pool" "poolseq" )

for region in "${regions[@]}"
do
    for prog in "${progs[@]}"
    do
        python ../log_parse_sum.py ../../data/tembusu/$prog/particles/$region/$prog.log
    done
done
