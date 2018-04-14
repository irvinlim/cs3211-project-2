# Sample Specifications

This directory contains sample specifications, which will demonstrate some examples of how the program can be used.

## `collisiontest`

Demonstrates elastic collisions that work with the gravitational pull of 4 particles. Run the following command for execution on a single region:

```sh
mpirun -np 1 pool samplespec/collisiontest.txt finalbrd.ppm report/report.txt ./animator/frames/collisiontest
```

## `horizontest`

Demonstrates that gravitational pull and collisions are computed across regions. Note how the particles are attracted towards each other at the edges of each region. Run the following command for execution on 4 regions:

```sh
mpirun -np 4 pool samplespec/horizontest.txt finalbrd.ppm report/report.txt ./animator/frames/horizontest
```

## `overlaptest`

Demonstrates completely overlapping particles in their initial position (with zero velocity). The particles' velocities are modified arbitrarily, so that they can be "pushed back" away from each other in the opposite direction of their unit vector. Run the following command for execution on a single region:

```sh
mpirun -np 1 pool samplespec/overlaptest.txt finalbrd.ppm report/report.txt ./animator/frames/overlaptest
```
