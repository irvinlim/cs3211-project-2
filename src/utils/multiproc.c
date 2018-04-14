#include <math.h>
#include <mpi.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "multiproc.h"
#include "types.h"

#define MASTER_ID 0
#define PARTICLE_FIELD_COUNT 9

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
 * Initializes and creates a datatype for the Particle struct.
 */
void mpi_init_particle(MPI_Datatype *newtype)
{
    int blocklengths[PARTICLE_FIELD_COUNT] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };

    MPI_Aint displacements[PARTICLE_FIELD_COUNT] = {
        offsetof(Particle, id),
        offsetof(Particle, region),
        offsetof(Particle, size),
        offsetof(Particle, mass),
        offsetof(Particle, radius),
        offsetof(Particle, x),
        offsetof(Particle, y),
        offsetof(Particle, vx),
        offsetof(Particle, vy),
    };

    MPI_Datatype types[PARTICLE_FIELD_COUNT] = {
        MPI_INT,
        MPI_INT,
        MPI_INT,
        MPI_LONG_DOUBLE,
        MPI_LONG_DOUBLE,
        MPI_LONG_DOUBLE,
        MPI_LONG_DOUBLE,
        MPI_LONG_DOUBLE,
        MPI_LONG_DOUBLE,
    };

    // Create the datatype.
    int error = MPI_Type_create_struct(PARTICLE_FIELD_COUNT, blocklengths, displacements, types, newtype);
    if (error != MPI_SUCCESS) {
        LL_ERROR("Could not initialize Particle MPI datatype! Error code: %d", error);
        exit(EXIT_FAILURE);
    }

    MPI_Type_commit(newtype);
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
 * Very simple wrapper around MPI_Send, which adds debug messages.
 */
int mpi_send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    LL_MPI2("Sending %d items to process %d...", count, dest);
    int error = MPI_Send(buf, count, datatype, dest, tag, comm);
    LL_MPI2("Sent %d items to process %d!", count, dest);

    return error;
}

/**
 * Very simple wrapper around MPI_Recv, which adds debug messages.
 */
int mpi_recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    LL_MPI2("Receiving %d items from process %d...", count, source);
    int error = MPI_Recv(buf, count, datatype, source, tag, comm, status);
    LL_MPI2("Received %d items from process %d!", count, source);

    return error;
}
