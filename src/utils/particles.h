#include "types.h"

/**
 * Generate both small and large particles for a single region, according to the given spec.
 * The positions of the small particles will be randomized anywhere within the region.
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
