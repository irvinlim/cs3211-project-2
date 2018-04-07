#include "types.h"

/**
 * Generate all particles based on the provided specifications.
 */
Particle *generate_particles(Spec spec);

/**
 * Generate a heatmap of particles and saves it to a file.
 */
void generate_heatmap(Spec spec, Particle *particles, char *outputfile);

/**
 * Debug prints details about a particle.
 */
void print_particle(Particle particle);

/**
 * Debug prints all particles.
 */
void print_particles(int n, Particle *particles);

/**
 * Prints a concatenated list of all particle IDs.
 */
void print_particle_ids(char *msg, int n, Particle *particles);
