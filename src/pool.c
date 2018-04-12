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

#include <assert.h>
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

// Store the computation and communication time for each iteration.
long long *comm_time;
long long *comp_time;

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

    /// Step 3: Truncate the sizes for all regions except the one that received all particles from.
    for (int region = 0; region < num_cores; region++) {
        if (region != my_region)
            sizes[region] = 0;
        else
            sizes[region] = total_sizes[region];
    }

    /// Step 4: Duplicate the final particles to other horizon processes which will need it.
    for (int receiver = 0; receiver < num_cores; receiver++) {
        for (int sender = 0; sender < num_cores; sender++) {
            if (sender == receiver) continue;
            if (my_region != sender && my_region != receiver) continue;

            // Check the horizon distance between any two pairs of regions.
            int horizon_dist = get_horizon_dist(spec.PoolLength, sender, receiver);
            assert(horizon_dist >= 0);
            if (horizon_dist > spec.Horizon) continue;

            // Send the particles.
            if (sender == my_region) {
                LL_MPI("Sending %d particles to %d", sizes[my_region], receiver);
                mpi_send(final_particles[my_region], sizes[my_region], mpi_particle_type, receiver, 0, MPI_COMM_WORLD);
            } else if (receiver == my_region) {
                LL_MPI("Receiving %d particles from %d", total_sizes[my_region], sender);
                mpi_recv(final_particles[sender], total_sizes[sender], mpi_particle_type, sender, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }

            // Update the size as well.
            sizes[sender] = total_sizes[sender];
        }
    }

    /// Complete!

    if (is_master()) LL_VERBOSE("Particle synchronisation is complete between %d processes.", num_cores);
    print_ints(LOG_LEVEL_MPI, "Final region sizes for this process", num_cores, sizes);
    print_particles(LOG_LEVEL_MPI, "Particles in my region after synchronisation", sizes[my_region], final_particles[my_region]);

    // Free unused buffers.
    free(total_sizes);
    deallocate_particles(particles, num_cores);

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
    long long start, end;
    char timebuf[TIMEBUF_LENGTH];
    int region_id = get_process_id();

    // Initialise the timing arrays.
    comm_time = malloc(spec.TimeSlots * sizeof(long long));
    comp_time = malloc(spec.TimeSlots * sizeof(long long));

    for (long i = 0; i < spec.TimeSlots; i++) {
        // Synchronise particles, such that we send all particles that we computed,
        // and receive updated particles for all regions.
        start = wall_clock_time();
        particles_by_region = sync_particles(sizes, particles_by_region);
        end = wall_clock_time();
        comm_time[i] = end - start;
        format_time(timebuf, TIMEBUF_LENGTH, end - start);
        LL_VERBOSE("Communication time for iteration %4.0ld: %s seconds", i + 1, timebuf);

        // Execute time step.
        start = wall_clock_time();
        particles_by_region = execute_time_step(sizes, particles_by_region);
        end = wall_clock_time();
        comp_time[i] = end - start;
        format_time(timebuf, TIMEBUF_LENGTH, end - start);
        LL_VERBOSE("Computation time for iteration %4.0ld: %s seconds", i + 1, timebuf);

        // Wait for all processes to complete computation before proceeding.
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Synchronise particles one more time.
    particles_by_region = sync_particles(sizes, particles_by_region);
    MPI_Barrier(MPI_COMM_WORLD);

    // Get total and average timing for all iterations.
    long long comm_sum = 0;
    long long comp_sum = 0;

    for (int i = 0; i < spec.TimeSlots; i++) {
        comm_sum += comm_time[i];
        comp_sum += comp_time[i];
    }

    LL_VERBOSE("Computation time for region %d:", region_id);
    format_time(timebuf, TIMEBUF_LENGTH, comp_sum);
    LL_VERBOSE("+ Total: %s seconds", timebuf);
    format_time(timebuf, TIMEBUF_LENGTH, comp_sum / spec.TimeSlots);
    LL_VERBOSE("+ Average: %s seconds", timebuf);

    LL_VERBOSE("Communication time for region %d:", region_id);
    format_time(timebuf, TIMEBUF_LENGTH, comm_sum);
    LL_VERBOSE("+ Total: %s seconds", timebuf);
    format_time(timebuf, TIMEBUF_LENGTH, comm_sum / spec.TimeSlots);
    LL_VERBOSE("+ Average: %s seconds", timebuf);

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
 * Main method for the structue of the entire program.
 */
void start(char *specfile, char *outputfile)
{
    int *sizes;
    Particle **particles_by_region;
    int region_id = get_process_id();

    if (is_master()) LL_NOTICE("Starting %s with %d region(s) on %d processor(s)...", PROG, get_num_cores(), get_num_cores());

    // Read the specification file into a struct (in parallel).
    spec = read_spec_file(region_id, specfile);

    // Debug print all read-in values.
    if (is_master()) print_spec(spec);
    if (is_master()) print_canvas_info(spec);

    // Initialize arrays and generate particles.
    particles_by_region = init_particles(&sizes);
    MPI_Barrier(MPI_COMM_WORLD);

    // Run the simulation only in the region assigned.
    if (is_master()) LL_NOTICE("Simulation is starting on %d core(s).", get_num_cores());
    particles_by_region = run_simulation(sizes, particles_by_region);
    MPI_Barrier(MPI_COMM_WORLD);
    if (is_master()) LL_NOTICE("%s", "Simulation completed!");

    // Collate timings from all processes to generate a report.
    collate_timings();

    // Collate particles and generate the heatmap on the master process.
    collate_generate_heatmap(sizes, particles_by_region, outputfile);
}

int main(int argc, char **argv)
{
    multiproc_init(argc, argv);
    set_log_level_env();
    mpi_init_particle(&mpi_particle_type);
    check_arguments(argc, argv, PROG);

    start(argv[1], argv[2]);

    multiproc_finalize();
    return 0;
}
