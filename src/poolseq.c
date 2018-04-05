#include <stdio.h>
#include <stdlib.h>

#include "utils/common.h"
#include "utils/log.h"
#include "utils/multiproc.h"
#include "utils/particles.h"
#include "utils/spec.h"
#include "utils/timer.h"

#define PROG "poolseq"

/**
 * Runs the simulation according to the provided specifications.
 */
void run_simulation(Spec spec, Particle *particles)
{
    char time_passed[10];
    long long simul_start = wall_clock_time();
    LL_NOTICE("Simulation is starting on %d core(s).", 1);

    for (long i = 0; i < spec.TimeSlots; i++) {
        // Start timer.
        long long start = wall_clock_time();

        // TODO: Run simulation

        // Get timing and log.
        get_time_passed(time_passed, start);
        LL_VERBOSE("Completed iteration %ld in %s seconds.", i + 1, time_passed);
    }

    // Get timing and log.
    get_time_passed(time_passed, simul_start);
    LL_NOTICE("Simulation complete: %ld iteration(s) performed in %s seconds.", spec.TimeSlots, time_passed);
}

/**
 * For the sequential version of the program, we will run the 
 * entire procedure on a single process, although the number of
 * cores (supplied by the -np flag) will be used to determine the
 * number of regions that should be computed.
 */
int main(int argc, char **argv)
{
    // Initialize MPI.
    multiproc_init(argc, argv);

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

    // Clean up.
    multiproc_finalize();
    return 0;
}
