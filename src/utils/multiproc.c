#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "multiproc.h"

#define MASTER_ID 0

/**
 * MPI rank number.
 */
int rank;

/**
 * MPI size.
 */
int size;

/**
 * Initializes MPI.
 */
void multiproc_init(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}

/**
 * Finalizes MPI.
 */
void multiproc_finalize()
{
    MPI_Finalize();
}

/**
 * Gets the number of cores.
 */
int get_num_cores()
{
    // Fail if the number of cores is not a perfect square.
    int np_sqrt = sqrt(size);

    if (np_sqrt * np_sqrt != size) {
        LL_ERROR("Number of processes %d must be a perfect square!", size);
        exit(EXIT_FAILURE);
    }

    return size;
}

/**
 * Gets the process ID.
 */
int get_process_id()
{
    return rank;
}

/**
 * Returns 1 if the current process is master.
 */
int is_master()
{
    return get_process_id() == MASTER_ID;
}

/**
 * Creates a MPI barrier to wait for all processes.
 */
void wait_barrier()
{
    MPI_Barrier(MPI_COMM_WORLD);
}
