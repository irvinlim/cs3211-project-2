#include "../utils/types.h"

/**
 * Returns the region that a particle resides in.
 */
int get_region(Particle p, Spec spec);

/**
 * Returns the x-coordinate of a region.
 */
int get_region_x(int region_id, Spec spec);

/**
 * Returns the y-coordinate of a region.
 */
int get_region_y(int region_id, Spec spec);

/**
 * Denormalizes an x-coordinate wrt region.
 */
int denorm_region_x(long double x, int region_id, Spec spec);

/**
 * Denormalizes an y-coordinate wrt region.
 */
int denorm_region_y(long double y, int region_id, Spec spec);

/**
 * Returns the horizon distance between two region IDs, 
 * relative to the number of regions (provided by pool_length).
 */
int get_horizon_dist(int pool_length, int r1, int r2);

/**
 * Allocates space for a 2-D array of arrays of particles, indexed by region.
 */
Particle **allocate_particles(int *sizes, int n_regions);

/**
 * Allocates space for as many particles as there are for each region.
 */
Particle **allocate_max_particles(int max_size, int n_regions);

/**
 * Filters an array of particles by region.
 */
Particle *filter_by_region(int *n_filtered, Spec spec, int region_id, Particle *p, int n);
