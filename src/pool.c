/**
 * pool.c
 * 
 * For the parallel version of the program, the number of regions
 * will be taken from the number of processes (as indicated through
 * the -np flag), as expected.
 * 
 * This variant of the program uses the following communication strategy:
 * TODO: FILL THIS UP
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simulation/nbody.h"
#include "simulation/regions.h"
#include "utils/common.h"
#include "utils/log.h"
#include "utils/multiproc.h"
#include "utils/particles.h"
#include "utils/spec.h"
#include "utils/timer.h"

#define PROG "pool"
#define TIMEBUF_LENGTH 10

// Stores the specifications for the program.
Spec spec;

// Custom MPI datatype to store our Particle struct.
MPI_Datatype mpi_particle_type;

/**
 * Initialize arrays of particles and generate the initial particles
 * to be located entirely in the region ID corresponding to the current process ID.
 */
Particle **init_particles(int **sizes)
{
    int num_cores = get_num_cores();
    int region_id = get_process_id();

    // Allocate space for particles and their array sizes.
    *sizes = (int *)calloc(num_cores, sizeof(int));
    (*sizes)[region_id] = spec.TotalNumberOfParticles;
    Particle **particles = allocate_particles(*sizes, num_cores);

    // Generate particles for __this region only__.
    particles[region_id] = generate_particles(spec);

    return particles;
}

/**
 * Synchronises particles with other processes.
 * 
 * @param send_sizes                Sizes of array of particles to send, corresponding to each region.
 * @param send_particles_by_region  2-D array of particles, indexed by region ID.
 *                                  This array should only contain the particles computed by this processor.
 * @return                          2-D array of particles, indexed by region ID.
 *                                  This array should contain the updated particles after synchronisation.
 */
Particle **sync_particles(int *send_sizes, Particle **send_particles_by_region)
{
    int num_cores = get_num_cores();
    int my_region = get_process_id();

    /// This processor needs to send the particles it computed to other processors.
    /// Other processors needs to receive the particles for its region, as well as for the horizon regions.

    /// Step 1: Determine the total number of particles located in each region across all processes.

    // Initialize an array to store the final sizes for all regions.
    int *final_sizes = calloc(num_cores, sizeof(int));

    // This will sum up all items in the send_sizes array across all processes.
    // Note that the process doesn't necessarily need to know the size of every other region,
    // but since the data being sent is small enough we can afford to use Allreduce.
    MPI_Allreduce(send_sizes, final_sizes, num_cores, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // Debug logging.
    char *final_size_str = malloc(num_cores * 12);
    join_ints(final_size_str, ',', num_cores, final_sizes);
    LL_MPI("Process %d: Final sizes of regions - %s", my_region, final_size_str);
    if (is_master()) LL_MPI("%s", "Make sure that all regions have the same final sizes.");

    // Allocate space in the final_particles array, based on the sizes we calculated earlier.
    Particle **final_particles = allocate_particles(final_sizes, num_cores);

    /// Step 2: Send the particles that should belong to a particular region to that process.

    // Whatever this processor computed for its own region, we can keep.
    memcpy(final_particles[my_region], send_particles_by_region[my_region], final_sizes[my_region] * sizeof(Particle));

    // Each process can receive particles (for the same region) from __any__ process.
    // This is because a process can compute a particle that ends up in a different process.
    int total_received_size = 0;
    for (int region = 0; region < num_cores; region++) {
        // Loop through all regions __in order__, and see if it is this processor's turn to be sending particles.

        if (region == my_region) {
            /// My turn: Send to all regions in order.

            // Loop through all regions to send to.
            for (int dest = 0; dest < num_cores; dest++) {
                // Don't need to send to yourself.
                if (dest == my_region) continue;

                // First send the size of the subarray we are going to send.
                mpi_send(&send_sizes[dest], 1, MPI_INT, dest, 0, MPI_COMM_WORLD);

                LL_MPI("Process %d: About to send %d particles to process %d.", my_region, send_sizes[dest], dest);

                // Now we can send the array of particles that __belongs to the process' region__.
                mpi_send(send_particles_by_region[dest], send_sizes[dest], mpi_particle_type, dest, 0, MPI_COMM_WORLD);
            }
        } else {
            /// Not my turn: Receive from other regions in order.

            // First receive the size of the subarray we are going to receive.
            int subarray_recv_size;
            mpi_recv(&subarray_recv_size, 1, MPI_INT, region, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            LL_MPI("Process %d: About to receive %d particles from process %d.", my_region, subarray_recv_size, region);

            // Receive the particles from the other process that __belongs to my process' region__.
            mpi_recv(&final_particles[my_region][total_received_size], subarray_recv_size, mpi_particle_type, region, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Increment the size received so far.
            total_received_size += subarray_recv_size;
        }
    }

    // Debug logging.
    int *final_particle_ids = malloc(final_sizes[my_region] * sizeof(int));
    for (int i = 0; i < final_sizes[my_region]; i++) final_particle_ids[i] = final_particles[my_region][i].id;
    char *final_particles_str = malloc(num_cores * 12);
    join_ints(final_particles_str, ',', final_sizes[my_region], final_particle_ids);
    LL_MPI("Process %d: Final IDs for region %d - %s", my_region, my_region, final_particles_str);

    /// Step 3: Duplicate the final particles to other horizon processes which will need it.

    // TODO

    /// Step 4: Truncate the sizes for regions we did not update in Step 3.

    // TODO: Should not truncate horizon regions once we sync horizon regions.
    for (int region = 0; region < num_cores; region++) {
        if (region != my_region)
            send_sizes[region] = 0;
        else
            send_sizes[region] = final_sizes[region];
    }

    /// Complete!

    if (is_master()) LL_VERBOSE("Particle synchronization is complete between %d processes.", num_cores);
    print_particles(LOG_LEVEL_MPI, send_sizes[my_region], final_particles[my_region]);

    return final_particles;
}

/**
 * Runs a single time step.
 */
Particle **execute_time_step(int *sizes, Particle **particles_by_region)
{
    int num_cores = get_num_cores();
    int region_id = get_process_id();
    long double dt = spec.TimeStep;

    // Compute the new velocities for all particles in the region that this process is computing for,
    // taking particles in other regions as part of the computation.
    update_velocity(dt, spec, num_cores, particles_by_region, sizes, region_id);

    // Update the position and region of all particles, for the particles in the region
    // that this process is computing for.
    particles_by_region = update_position_and_region(dt, spec, num_cores, particles_by_region, sizes, region_id);

    return particles_by_region;
}

/**
 * Runs the simulation according to the provided specifications.
 */
void run_simulation(int *sizes, Particle **particles_by_region)
{
    char time_passed[TIMEBUF_LENGTH];
    int region_id = get_process_id();

    for (long i = 0; i < spec.TimeSlots; i++) {
        // Start timer.
        long long start = wall_clock_time();

        // Synchronise particles, such that we send all particles that we computed,
        // and receive updated particles for all regions.
        particles_by_region = sync_particles(sizes, particles_by_region);

        // Execute time step.
        particles_by_region = execute_time_step(sizes, particles_by_region);

        // Get timing and log.
        long long end = wall_clock_time();
        format_time(time_passed, TIMEBUF_LENGTH, end - start);
        LL_VERBOSE("Region %d: Completed iteration %4.0ld in %s seconds", region_id, i + 1, time_passed);
    }
}

/**
 * Collates timings for all processes, calculates the average
 * and generates a report.
 */
void collate_timings()
{
    // TODO
}

/**
 * Only executed by the master process.
 * 
 * NOTE: There is no master-slave communication for the main simulation computation,
 * but in order to streamline log messages, only the master process should write
 * to the output buffer.
 */
void master(char *specfile, char *outputfile)
{
    int *sizes;
    Particle **particles_by_region;

    // Read the specification file into a struct (in parallel).
    spec = read_spec_file(specfile);

    // Debug print all read-in values.
    print_spec(spec);
    print_canvas_info(spec);

    // Initialize arrays and generate particles.
    particles_by_region = init_particles(&sizes);

    // Wait until all processes have generated particles.
    MPI_Barrier(MPI_COMM_WORLD);

    LL_NOTICE("Simulation is starting on %d core(s).", get_num_cores());

    // Run the simulation only in the region assigned.
    run_simulation(sizes, particles_by_region);

    // Master only: Get timings for all processes to generate report.
    collate_timings();

    // Master only: Generate the heatmap.
    // TODO: Collate particles from all regions into a single array.
    // generate_heatmap(spec, particles, outputfile);
}

/**
 * Only executed by slave processes.
 */
void slave(char *specfile)
{
    int *sizes;
    Particle **particles_by_region;

    // Read the specification file into a struct (in parallel).
    spec = read_spec_file(specfile);

    // Initialize arrays and generate particles.
    particles_by_region = init_particles(&sizes);

    // Wait until all processes have generated particles.
    MPI_Barrier(MPI_COMM_WORLD);

    // Run the simulation only in the region assigned.
    run_simulation(sizes, particles_by_region);
}

int main(int argc, char **argv)
{
    multiproc_init(argc, argv);
    set_log_level_env();

    // Initialize custom MPI datatypes.
    mpi_init_particle(&mpi_particle_type);

    // Parse arguments.
    check_arguments(argc, argv, PROG);
    char *specfile = argv[1];
    char *outputfile = argv[2];

    if (is_master()) LL_NOTICE("Starting %s with %d region(s) on %d processor(s)...", PROG, get_num_cores(), get_num_cores());

    // Differentiate work on master vs slaves.
    if (is_master())
        master(specfile, outputfile);
    else
        slave(specfile);

    multiproc_finalize();
    return 0;
}
