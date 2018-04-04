#include "types.h"
#include "common.h"

#define DEBUG_PARTICLES 0

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
void print_particles(Particle *particles);
