#include "../utils/types.h"

/**
 * Returns the region that a particle resides in.
 */
int get_region(Particle p, Spec spec);

/**
 * Returns the horizon distance between two region IDs, 
 * relative to the number of regions (provided by pool_length).
 */
int get_horizon_dist(int pool_length, int r1, int r2);

/**
 * Filters an array of particles by region.
 */
Particle *filter_by_region(int *n_filtered, Spec spec, int region_id, Particle *p, int n);
