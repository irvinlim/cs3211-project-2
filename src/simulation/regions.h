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
