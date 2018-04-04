#include "types.h"
#include "common.h"

#define DEBUG 1

/**
 * Generate all particles based on the provided specifications.
 */
Particle *generate_particles(Spec spec);

/**
 * Debug prints all particles.
 */
void print_particles(Particle *particles);

/**
 * Debug prints details about a particle.
 */
void print_particle(Particle particle);
