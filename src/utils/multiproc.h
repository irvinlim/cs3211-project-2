#include <mpi.h>

/**
 * Initializes MPI.
 */
void multiproc_init(int argc, char **argv);

/**
 * Initializes and creates a datatype for the Particle struct.
 */
void mpi_init_particle(MPI_Datatype *newtype);

/**
 * Finalizes MPI.
 */
void multiproc_finalize();

/**
 * Gets the number of cores.
 */
int get_num_cores();

/**
 * Gets the process ID.
 */
int get_process_id();

/**
 * Returns 1 if the current process is master.
 */
int is_master();

/**
 * Very simple wrapper around MPI_Send, which adds debug messages.
 */
int mpi_send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

/**
 * Very simple wrapper around MPI_Recv, which adds debug messages.
 */
int mpi_recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
