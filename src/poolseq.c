#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/common.h"
#include "utils/particles.h"

#define PROG "poolseq"

/**
 * Runs the simulation according to the provided specifications.
 */
void run_simulation(Particle *particles, Spec spec)
{
}

int main(int argc, char **argv)
{
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

    // Clean up.
    exit(EXIT_SUCCESS);
}
