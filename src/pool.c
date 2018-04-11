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
#include "utils/common.h"
#include "utils/heatmap.h"
#include "utils/log.h"
#include "utils/multiproc.h"
#include "utils/particles.h"
#include "utils/regions.h"
#include "utils/spec.h"
#include "utils/timer.h"

#define PROG "pool"
#define TIMEBUF_LENGTH 10
#define MASTER_ID 0

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
    Particle **particles = allocate_particles(*sizes, num_cores);

    // Generate particles for __this region only__.
    particles[region_id] = generate_particles(region_id, spec);
    (*sizes)[region_id] = spec.TotalNumberOfParticles;

    return particles;
}

/**
 * Synchronises particles with other processes.
 * 
 * @param sizes         Sizes of array of particles to send, corresponding to each region.
 * @param particles     2-D array of particles, indexed by region ID.
 *                      This array should only contain the particles computed by this processor.
 * @return              2-D array of particles, indexed by region ID.
 *                      This array should contain the updated particles after synchronisation.
 */
Particle **sync_particles(int *sizes, Particle **particles)
{
    int num_cores = get_num_cores();
    int my_region = get_process_id();

    /// This processor needs to send the particles it computed to other processors.
    /// Other processors needs to receive the particles for its region, as well as for the horizon regions.
    LL_MPI("%s", "Synchronising particles...");

    /// Step 1: Determine the total number of particles located in each region across all processes.

    // Initialize an array to store the total sizes for all regions.
    int *total_sizes = calloc(num_cores, sizeof(int));

    // This will sum up all items in the sizes array across all processes.
    // Note that the process doesn't necessarily need to know the size of every other region,
    // but since the data being sent is small enough we can afford to use Allreduce.
    print_ints(LOG_LEVEL_MPI, "Region sizes that I am sending", num_cores, sizes);
    MPI_Allreduce(sizes, total_sizes, num_cores, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    print_ints(LOG_LEVEL_MPI, "Total region sizes across all processes", num_cores, total_sizes);

    // Allocate space in the final_particles array, based on the sizes we calculated earlier.
    Particle **final_particles = allocate_particles(total_sizes, num_cores);

    /// Step 2: Send the particles that should belong to a particular region to that process.

    // Whatever this processor computed for its own region, we can keep.
    memcpy(final_particles[my_region], particles[my_region], sizes[my_region] * sizeof(Particle));

    // Each process can receive particles (for the same region) from __any__ process.
    // This is because a process can compute a particle that ends up in a different process.
    int receive_offset = sizes[my_region];
    for (int region = 0; region < num_cores; region++) {
        // Loop through all regions __in order__, and see if it is this processor's turn to be sending particles.

        if (region == my_region) {
            /// My turn: Send to all regions in order.

            // Loop through all regions to send to.
            for (int dest = 0; dest < num_cores; dest++) {
                // Don't need to send to yourself.
                if (dest == my_region) continue;

                // First send the size of the subarray we are going to send.
                mpi_send(&sizes[dest], 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
                LL_MPI("About to send %d particles to process %d.", sizes[dest], dest);

                // Now we can send the array of particles that __belongs to the process' region__.
                mpi_send(particles[dest], sizes[dest], mpi_particle_type, dest, 0, MPI_COMM_WORLD);
            }
        } else {
            /// Not my turn: Receive from other regions in order.

            // First receive the size of the subarray we are going to receive.
            int subarray_recv_size;
            mpi_recv(&subarray_recv_size, 1, MPI_INT, region, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            LL_MPI("About to receive %d particles from process %d.", subarray_recv_size, region);

            // Receive the particles from the other process that __belongs to my process' region__.
            mpi_recv(&final_particles[my_region][receive_offset], subarray_recv_size, mpi_particle_type, region, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Increment the size received so far.
            receive_offset += subarray_recv_size;
        }
    }

    // Debug logging.
    print_particle_ids(LOG_LEVEL_MPI, "Final IDs for my region", total_sizes[my_region], final_particles[my_region]);

    /// Step 3: Duplicate the final particles to other horizon processes which will need it.

    // TODO

    /// Step 4: Truncate the sizes for regions we did not update in Step 3.

    // TODO: Should not truncate horizon regions once we sync horizon regions.
    for (int region = 0; region < num_cores; region++) {
        if (region != my_region)
            sizes[region] = 0;
        else
            sizes[region] = total_sizes[region];
    }

    print_ints(LOG_LEVEL_MPI, "Final sizes for this process", num_cores, sizes);

    /// Complete!

    if (is_master()) LL_VERBOSE("Particle synchronisation is complete between %d processes.", num_cores);
    print_particles(LOG_LEVEL_MPI, "Particles in my region after synchronisation", sizes[my_region], final_particles[my_region]);

    // Free unused buffers.
    free(total_sizes);

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
    Particle **updated_particles = update_position_and_region(dt, spec, num_cores, particles_by_region, sizes, region_id);

    return updated_particles;
}

/**
 * Runs the simulation according to the provided specifications.
 */
Particle **run_simulation(int *sizes, Particle **particles_by_region)
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

        // Wait for all processes to complete computation.
        MPI_Barrier(MPI_COMM_WORLD);

        // Get timing and log.
        long long end = wall_clock_time();
        format_time(time_passed, TIMEBUF_LENGTH, end - start);
        LL_VERBOSE("Region %d: Completed iteration %4.0ld in %s seconds", region_id, i + 1, time_passed);
    }

    // Synchronise particles one more time.
    particles_by_region = sync_particles(sizes, particles_by_region);
    MPI_Barrier(MPI_COMM_WORLD);

    return particles_by_region;
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
 * Generates and collates canvases from all processes into the
 * master process, so that we can generate a PPM heatmap.
 * 
 * Assumes that particles are already distributed across processes
 * into their own regions already.
 */
void collate_generate_heatmap(int *sizes, Particle **particles_by_region, char *outputfile)
{
    int num_cores = get_num_cores();
    int region_id = get_process_id();

    // Allocate space for a 3-D array.
    int ***collated = malloc(num_cores * sizeof(int **));
    for (int region = 0; region < num_cores; region++) {
        collated[region] = malloc(spec.GridSize * sizeof(int *));
        for (int y = 0; y < spec.GridSize; y++)
            collated[region][y] = malloc(spec.GridSize * sizeof(int));
    }

    // Generate canvas in the region that this process is in charge of.
    print_particles(LOG_LEVEL_DEBUG, "Generating canvas for my region", sizes[region_id], particles_by_region[region_id]);
    int **canvas = generate_region_canvas(spec.GridSize, sizes[region_id], particles_by_region[region_id]);
    LL_VERBOSE("Canvas generated for region %d.", region_id);

    // Send all canvases to the master process.
    for (int region = 0; region < num_cores; region++) {
        // Send each row of the canvas separately.
        for (int y = 0; y < spec.GridSize; y++) {
            // Only the P-th process should be sending to master.
            if (region == region_id)
                mpi_send(canvas[y], spec.GridSize, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD);

            // Handle receiving from P processes.
            if (is_master())
                mpi_recv(collated[region][y], spec.GridSize, MPI_INT, region, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    if (is_master()) LL_VERBOSE("%s", "All canvases sent to master!");

    // Generate the heatmap.
    if (is_master()) generate_heatmap(spec, collated, outputfile);

    // Free memory.
    for (int region = 0; region < num_cores; region++) {
        for (int y = 0; y < spec.GridSize; y++)
            free(collated[region][y]);
        free(collated[region]);
    }

    free(collated);
    free(canvas);
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
    int region_id = get_process_id();

    // Read the specification file into a struct (in parallel).
    spec = read_spec_file(region_id, specfile);

    // Debug print all read-in values.
    print_spec(spec);
    print_canvas_info(spec);

    // Initialize arrays and generate particles.
    particles_by_region = init_particles(&sizes);
    MPI_Barrier(MPI_COMM_WORLD);

    // Run the simulation only in the region assigned.
    LL_NOTICE("Simulation is starting on %d core(s).", get_num_cores());
    run_simulation(sizes, particles_by_region);
    MPI_Barrier(MPI_COMM_WORLD);
    LL_NOTICE("%s", "Simulation completed!");

    // Master only: Get timings for all processes to generate report.
    collate_timings();

    // Collate particles and generate the heatmap on the master process.
    collate_generate_heatmap(sizes, particles_by_region, outputfile);
}

/**
 * Only executed by slave processes.
 */
void slave(char *specfile)
{
    int region_id = get_process_id();
    int *sizes;
    Particle **particles_by_region;

    // Read the specification file into a struct (in parallel).
    spec = read_spec_file(region_id, specfile);

    // Initialize arrays and generate particles.
    particles_by_region = init_particles(&sizes);

    // Wait until all processes have generated particles.
    MPI_Barrier(MPI_COMM_WORLD);

    // Run the simulation only in the region assigned.
    run_simulation(sizes, particles_by_region);
    MPI_Barrier(MPI_COMM_WORLD);

    // Collate particles and generate the heatmap on the master process.
    collate_generate_heatmap(sizes, particles_by_region, NULL);
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
