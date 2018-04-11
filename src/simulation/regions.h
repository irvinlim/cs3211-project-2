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
long double denorm_region_x(long double x, int region_id, Spec spec);

/**
 * Denormalizes an y-coordinate wrt region.
 */
long double denorm_region_y(long double y, int region_id, Spec spec);

/**
 * Normalizes a coordinate wrt region.
 */
long double norm_region(long double coord, Spec spec);

/**
 * Wraps around a coordinate.
 */
long double wrap_around(long double coord, int max_coord);

/**
 * Returns the horizon distance between two region IDs, 
 * relative to the number of regions (provided by pool_length).
 */
int get_horizon_dist(int pool_length, int r1, int r2);
