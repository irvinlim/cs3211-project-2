#include <stdio.h>
#include <stdlib.h>

#include "../utils/types.h"

/**
 * Returns the region that a particle resides in.
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
 * Returns the horizon distance between two region IDs, 
 * relative to the number of regions (provided by pool_length).
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
