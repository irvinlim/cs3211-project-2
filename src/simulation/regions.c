#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/types.h"

/**
 * Returns the region that a particle resides in.
 * 
 * TODO: Test if function is working.
 */
int get_region(Particle p, Spec spec)
{
    int region_x = p.x / spec.GridSize;
    int region_y = p.y / spec.GridSize;

    // Limit the region IDs to be bounded to the corner regions, just in case.
    // Particles should be wrapped around when they exceed the boundaries of the canvas.
    region_x = region_x < 0 ? 0 : region_x >= spec.PoolLength ? spec.PoolLength - 1 : region_x;
    region_y = region_y < 0 ? 0 : region_y >= spec.PoolLength ? spec.PoolLength - 1 : region_y;

    return region_y * spec.PoolLength + region_x;
}

/**
 * Returns the x-coordinate of a region.
 */
int get_region_x(int region_id, Spec spec)
{
    return region_id % spec.PoolLength;
}

/**
 * Returns the y-coordinate of a region.
 */
int get_region_y(int region_id, Spec spec)
{
    return region_id / spec.PoolLength;
}

/**
 * Denormalizes an x-coordinate wrt region.
 */
long double denorm_region_x(long double x, int region_id, Spec spec)
{
    return x + get_region_x(region_id, spec) * spec.GridSize;
}

/**
 * Denormalizes an y-coordinate wrt region.
 */
long double denorm_region_y(long double y, int region_id, Spec spec)
{
    return y + get_region_y(region_id, spec) * spec.GridSize;
}

/**
 * Normalizes a coordinate wrt region.
 */
long double norm_region(long double coord, Spec spec)
{
    return fmodl(coord, spec.GridSize);
}

/**
 * Wraps around a coordinate.
 */
long double wrap_around(long double coord, int max_coord)
{
    while (coord < 0 || coord >= max_coord) {
        if (coord < 0)
            coord += max_coord;
        else if (coord >= max_coord)
            coord -= max_coord;
    }

    return coord;
}

/**
 * Returns the horizon distance between two region IDs, 
 * relative to the number of regions (provided by pool_length).
 * 
 * TODO: Test if function is working.
 */
int get_horizon_dist(int pool_length, int r1, int r2)
{
    // Enumerate outwards until we find region2.
    for (int i = 0; i <= pool_length; i++) {
        // Get the corners.
        int tl = r1 - pool_length * i - i;
        int tr = r1 - pool_length * i + i;
        int bl = r1 + pool_length * i - i;
        int br = r1 + pool_length * i + i;

        // Check corners first.
        if (tl == r2 || tr == r2 || bl == r2 || br == r2) return i;

        // Check top/bottom edges.
        if ((tl < r2 && r2 < tr) || (bl < r2 && r2 < br)) return i;

        // Check left/right edges.
        for (int j = tl; j <= bl; j += pool_length)
            if (j == r2) return i;
        for (int j = tr; j <= br; j += pool_length)
            if (j == r2) return i;
    }

    return -1;
}
