#!/bin/sh

regions=( 0 1 2 3 4 5 )
progs=( "pool" )

for region in "${regions[@]}"
do
    for prog in "${progs[@]}"
    do
        python ../log_parse.py ../../data/tembusu/$prog/horizon/$region/$prog.log
    done
done
