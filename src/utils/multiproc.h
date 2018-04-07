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
 * Creates a MPI barrier to wait for all processes.
 */
void wait_barrier();
