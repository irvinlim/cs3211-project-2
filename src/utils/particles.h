#include "types.h"
#include "common.h"

#define DEBUG_PARTICLES 1

/**
 * Generate all particles based on the provided specifications.
 */
Particle *generate_particles(Spec spec);

/**
 * Generate a heatmap of particles and saves it to a file.
 */
void generate_heatmap(Spec spec, Particle *particles, char *outputfile);

/**
 * Debug prints all particles.
 */
void print_particles(long long n, Particle *particles);
