#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/common.h"
#include "utils/log.h"
#include "utils/multiproc.h"
#include "utils/particles.h"

#define PROG "poolseq"

/**
 * Runs the simulation according to the provided specifications.
 */
void run_simulation(Particle *particles, Spec spec)
{
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
        LL_NOTICE("Starting %s with %d regions on 1 processor...", PROG, get_num_cores());

        // Parse arguments.
        check_arguments(argc, argv, PROG);
        char *specfile = argv[1];
        char *outputfile = argv[2];

        // Read the specification file into a struct.
        Spec spec = read_spec_file(specfile);

        // Generate the particles.
        Particle *particles = generate_particles(spec);

        // Run the simulation.
        run_simulation(particles, spec);

        // Generate the heatmap.
        generate_heatmap(spec, particles, outputfile);
    }

    // Clean up.
    multiproc_finalize();
    return 0;
}
