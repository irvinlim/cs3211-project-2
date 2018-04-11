# CS3211 Project 2

_Submitted by Irvin Lim Wei Quan (A0139812A)._

This is a submission for CS3211 Parallel and Concurrent Programming, Project 2 (_Alternate Physics: The Galactic Pool Table_).

The complete project report can also be found in the root directory.

## Installation

This project uses OpenMPI, and is known to work on at least version 3.0.1. You can download the OpenMPI library from [here](https://www.open-mpi.org/software/ompi/v3.0/).

To build the program, a Makefile is included. Run `make` as such:

```sh
make clean
make
```

## Usage

You can then run the program by calling the binary executable with `mpirun`, where `initialspec.txt` is a path to the specification file, and `finalbrd.ppm` is the path of the output PPM image file:

```sh
mpirun -np 64 pool initialspec.txt finalbrd.ppm
```

See `initialspec-example.txt` for an example of how the specification file should look like.

Change the value of the `np` flag to specify the number of regions that should be simulated. For the sequential version, only the master process will be working, while the parallel version will split the work up amongst all processes.

As such, the value of `np` **must be a perfect square**, e.g. 1, 4, 9, 16, 25, etc., since the "pool table" is a square.

### Verbosity

You can increase the verbosity of the output by passing the `LOG_LEVEL` environment variable, according to the following list:

* **0 - NONE**: Suppress all output
* **1 - ERROR**: Only show errors
* **2 - SUCCESS**: Also show success messages
* **3 - NOTICE (Default)**: Also show notices
* **4 - VERBOSE**: Show some verbose messages
* **5 - MPI**: Show verbose messages as well as MPI traces between processes
* **6 - DEBUG**: Debug mode
* **7 - DEBUG2**: Additional debug messages

For example, to increase the verbosity to VERBOSE, you can do

```sh
LOG_LEVEL=4 mpirun -np 64 pool initialspec.txt finalbrd.ppm
```

### Logging for a single process

Because multiple processes may be trying to write to the output buffer (which is `stderr` for this program) at the same time, the log messages will often be interleaved which makes it hard to follow especially on higher log levels.

To help with this, you can limit the logging process to only a certain process so that all log messages are guaranteed to be in order with the `LOG_PROCESS` environment variable, such as follows:

```sh
LOG_LEVEL=6 LOG_PROCESS=0 mpirun -np 4 pool initialspec.txt finalbrd.ppm
```

### Variants

Some variants of the program are included, which are listed below. All of the binaries take in the same command-line arguments as described above.

* `pool`: Parallel version of the galactic pool simulator
* `poolseq`: Sequential version of the galactic pool simulator

## License

Copyright, Irvin Lim
