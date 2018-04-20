#!/usr/bin/env python

import sys
import re
import itertools
from argparse import ArgumentParser, FileType

def parse(text):
    lines = [line for line in text.split('\n') if line.startswith('[0;3')]
    lines = [re.match(r'\[0;3\dm\[.*\] \d* ~ *(.*)\[0m$', line).groups()[0][:-1] for line in lines]
    
    # Parse the maximum computation time for each iteration.
    comp_time_lines = [line for line in lines if line.startswith("Computation time for iteration")]
    slowest_comp_times = {}
    
    for line in comp_time_lines:
        groups = re.match(r"Computation time for iteration *(\d+): ([\d.]+) seconds", line).groups()
        iteration = int(groups[0])
        time = float(groups[1])

        if not iteration in slowest_comp_times:
            slowest_comp_times[iteration] = 0
        
        slowest_comp_times[iteration] = max(slowest_comp_times[iteration], time)
    
    # Parse the maximum communication time for each iteration.
    comm_time_lines = [line for line in lines if line.startswith("Communication time for iteration")]
    slowest_comm_times = {}
    
    for line in comm_time_lines:
        groups = re.match(r"Communication time for iteration *(\d+): ([\d.]+) seconds", line).groups()
        iteration = int(groups[0])
        time = float(groups[1])

        if not iteration in slowest_comm_times:
            slowest_comm_times[iteration] = 0
        
        slowest_comm_times[iteration] = max(slowest_comm_times[iteration], time)

    sums = itertools.zip_longest(slowest_comp_times.values(), slowest_comm_times.values())
    # print("Sum of comp/comm times, %d iterations:" % len(slowest_comp_times.keys()))
    print(','.join(["%0.6Lf" % ((comp or 0) + (comm or 0)) for (comp, comm) in sums]))

def main():
    parser = ArgumentParser()
    parser.add_argument('infile', default=sys.stdin, type=FileType('r'))
    args = parser.parse_args()

    text = args.infile.read()
    parse(text)


if __name__ == "__main__":
    main()
