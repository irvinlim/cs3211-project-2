/**
 * Initializes MPI.
 */
void multiproc_init(int argc, char **argv);

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
