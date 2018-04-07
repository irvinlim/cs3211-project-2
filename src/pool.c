/**
 * pool.c
 * 
 * For the parallel version of the program, the number of regions
 * will be taken from the number of processes (as indicated through
 * the -np flag), as expected.
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "simulation/nbody.h"
#include "utils/common.h"
#include "utils/log.h"
#include "utils/multiproc.h"
#include "utils/particles.h"
#include "utils/spec.h"
#include "utils/timer.h"

#define PROG "pool"
#define TIMEBUF_LENGTH 10

/**
 * Stores the specifications for the program.
 */
Spec spec;

/**
 * Runs a single time step.
 */
void execute_time_step(Particle *p)
{
    int region_id = get_process_id();
    int n = spec.TotalNumberOfParticles;
    long double dt = spec.TimeStep;
    int horizon = spec.Horizon;

    // Compute the new velocity for this time step, only for this region.
    update_velocity(spec, p, n, dt, region_id, horizon);

    // Update the position of all particles.
    update_position(spec, p, n, dt, region_id);
}

/**
 * Synchronises particles with other processes.
 * Each process sends the particles in its region, whilst receiving
 * particles from all other regions.
 */
Particle *sync_particles(Particle *particles)
{
    Particle *received = particles;

    // We want to make sure that all particles are ready before sending.
    wait_barrier();

    // TODO: Filter and serialize array of structs to be sent.
    // TODO: Broadcast the number of particles you want to send.
    // TODO: Send out only the particles that are in this region.
    // TODO: Receive many particles and combine them back into the buffer.

    // We want to make sure all particles are received before proceeding with simulation.
    wait_barrier();

    if (is_master()) LL_DEBUG("Particle synchronization is complete between %d processes.", get_num_cores());

    return received;
}

/**
 * Runs the simulation according to the provided specifications.
 */
void run_simulation(Particle *p)
{
    char time_passed[TIMEBUF_LENGTH];
    int region_id = get_process_id();

    for (long i = 0; i < spec.TimeSlots; i++) {
        // Start timer.
        long long start = wall_clock_time();

        // Sync particles between processes.
        p = sync_particles(p);

        // Execute time step.
        execute_time_step(p);

        // Get timing and log.
        long long end = wall_clock_time();
        format_time(time_passed, TIMEBUF_LENGTH, end - start);
        LL_VERBOSE("Region %d: Completed iteration %4.0ld in %s seconds", region_id, i + 1, time_passed);
    }
}

/**
 * Only executed by the master process.
 */
void master(char *outputfile)
{
    char time_passed[TIMEBUF_LENGTH];
    long long total_start = wall_clock_time();

    // Generate all particles.
    Particle *particles = generate_particles(spec);

    LL_NOTICE("Simulation is starting on %d core(s).", get_num_cores());

    // Run the simulation only in the region assigned.
    run_simulation(particles);

    // Get timing and log.
    long long total_time = wall_clock_time() - total_start;
    LL_NOTICE("Simulation of %ld iteration(s) completed.", spec.TimeSlots);
    format_time(time_passed, TIMEBUF_LENGTH, total_time);
    LL_NOTICE("Total running time: %s seconds", time_passed);
    format_time(time_passed, TIMEBUF_LENGTH, total_time / spec.TimeSlots);
    LL_NOTICE("Average time per iteration: %s seconds", time_passed);
    print_particles(spec.TotalNumberOfParticles, particles);

    // Generate the heatmap.
    generate_heatmap(spec, particles, outputfile);
}

/**
 * Only executed by the slave processes.
 */
void slave()
{
    // Generate the particles for this region only.
    Particle *particles = generate_particles(spec);

    // Run the simulation only in the region assigned.
    run_simulation(particles);
}

int main(int argc, char **argv)
{
    multiproc_init(argc, argv);
    set_log_level_env();

    // Parse arguments.
    check_arguments(argc, argv, PROG);
    char *specfile = argv[1];
    char *outputfile = argv[2];

    if (is_master()) LL_NOTICE("Starting %s with %d region(s) on %d processor(s)...", PROG, get_num_cores(), get_num_cores());

    // Read the specification file into a struct (in parallel).
    spec = read_spec_file(specfile);

    // Differentiate work on master vs slaves.
    if (is_master()) {
        // Debug print all read-in values.
        print_spec(spec);
        print_canvas_info(spec);

        master(outputfile);
    } else {
        slave();
    }

    multiproc_finalize();
    return 0;
}
