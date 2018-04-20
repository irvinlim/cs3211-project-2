#!/bin/sh

regions=( 50 100 200 400 600 800 1000 1200 1400 1600 1800 2000 )
progs=( "pool" "poolseq" )

for region in "${regions[@]}"
do
    for prog in "${progs[@]}"
    do
        python ../log_parse_sum.py ../../data/tembusu/$prog/gridsize/$region/$prog.log
    done
done
