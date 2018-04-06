#include <stdio.h>
#include <stdlib.h>

#include "simulation/nbody.h"
#include "utils/common.h"
#include "utils/log.h"
#include "utils/multiproc.h"
#include "utils/particles.h"
#include "utils/spec.h"
#include "utils/timer.h"

#define PROG "poolseq"
#define TIMEBUF_LENGTH 10

/**
 * Runs the simulation according to the provided specifications.
 */
void run_simulation(Spec spec, Particle *particles)
{
    char time_passed[TIMEBUF_LENGTH];
    long long total_time = 0;
    LL_NOTICE("Simulation is starting on %d core(s).", 1);

    for (long i = 0; i < spec.TimeSlots; i++) {
        // Start timer.
        long long start = wall_clock_time();

        // Compute the new velocity for this timestep.
        update_velocity(particles, spec.TotalNumberOfParticles, spec.TimeStep);

        // Update the position of all particles.
        update_position(particles, spec.TotalNumberOfParticles, spec.TimeStep);

        // Get timing and log.
        long long end = wall_clock_time();
        total_time += end - start;
        format_time(time_passed, TIMEBUF_LENGTH, end - start);
        LL_VERBOSE("Completed iteration %4.0ld in %s seconds", i + 1, time_passed);
    }

    // Get timing and log.
    LL_NOTICE("Simulation of %ld iteration(s) completed.", spec.TimeSlots);

    format_time(time_passed, TIMEBUF_LENGTH, total_time);
    LL_NOTICE("Total running time: %s seconds", time_passed);
    format_time(time_passed, TIMEBUF_LENGTH, total_time / spec.TimeSlots);
    LL_NOTICE("Average time per iteration: %s seconds", time_passed);

    print_particles(spec.TotalNumberOfParticles, particles);
}

/**
 * For the sequential version of the program, we will run the 
 * entire procedure on a single process, although the number of
 * cores (supplied by the -np flag) will be used to determine the
 * number of regions that should be computed.
 */
int main(int argc, char **argv)
{
    multiproc_init(argc, argv);
    set_log_level_env();

    // Only run on a single (master) process.
    if (is_master()) {
        LL_NOTICE("Starting %s with %d region(s) on %d processor(s)...", PROG, get_num_cores(), 1);

        // Parse arguments.
        check_arguments(argc, argv, PROG);
        char *specfile = argv[1];
        char *outputfile = argv[2];

        // Read the specification file into a struct.
        Spec spec = read_spec_file(specfile);

        // Generate the particles.
        Particle *particles = generate_particles(spec);

        // Run the simulation.
        run_simulation(spec, particles);

        // Generate the heatmap.
        generate_heatmap(spec, particles, outputfile);
    }

    multiproc_finalize();
    return 0;
}
