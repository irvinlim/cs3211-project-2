# CS3211 Project 2

_Submitted by Irvin Lim Wei Quan (A0139812A)._

This is a submission for CS3211 Parallel and Concurrent Programming, Project 2 (_Alternate Physics: The Galactic Pool Table_).

The complete project report can also be found in the root directory.

## Installation

This project uses OpenMPI, and is known to work on at least version 3.0.1. You can download the OpenMPI library from [here](https://www.open-mpi.org/software/ompi/v3.0/).

To build the program, a Makefile is included. Run `make` as such:

```sh
make clean && make
```

You can then run the program by calling the binary executable with `mpirun`:

```sh
mpirun ./pool initialspec.txt finalbrd.ppm -np 64
```

## License

Copyright, Irvin Lim
