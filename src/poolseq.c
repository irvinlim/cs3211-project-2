#include <stdio.h>
#include <stdlib.h>
#include "utils/common.h"

#define PROG "poolseq"

/**
 * Runs the simulation according to the provided specifications.
 */
void run_simulation(Spec spec, char *outputfile)
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

    // Run the simulation.
    run_simulation(spec, outputfile);

    // Clean up.
    exit(EXIT_SUCCESS);
}
