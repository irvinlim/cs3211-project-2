#include "log.h"
#include "types.h"

/**
 * Allocates space for a 2-D array of arrays of particles, indexed by region.
 */
Particle **allocate_particles(int *sizes, int n_regions);

/**
 * Deallocate space that was reserved for a 2-D array of particles.
 */
void deallocate_particles(Particle **particles, int n_regions);

/**
 * Generate both small and large particles for a single region, according to the given spec.
 * The positions of the small particles will be randomized anywhere within the region.
 */
Particle *generate_particles(int region_id, Spec spec);

/**
 * Debug prints details about a particle.
 */
void print_particle(LogLevel level, Particle particle);

/**
 * Debug prints all particles.
 */
void print_particles(LogLevel level, char *msg, int n, Particle *particles);

/**
 * Prints a concatenated list of all particle IDs.
 */
void print_particle_ids(LogLevel level, char *msg, int n, Particle *particles);
