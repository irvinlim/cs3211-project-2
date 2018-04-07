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
    long region_x = p.x / spec.GridSize;
    long region_y = p.y / spec.GridSize;

    // Limit the region IDs to be bounded to the corner regions, just in case.
    // Particles should be wrapped around when they exceed the boundaries of the canvas.
    region_x = region_x < 0 ? 0 : region_x >= spec.PoolLength ? spec.PoolLength - 1 : region_x;
    region_y = region_y < 0 ? 0 : region_y >= spec.PoolLength ? spec.PoolLength - 1 : region_y;

    return region_y * spec.PoolLength + region_x;
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

/**
 * Filters an array of particles by region.
 */
Particle *filter_by_region(int *n_filtered, Spec spec, int region_id, Particle *p, int n)
{
    // Allocate space for all particles first.
    Particle *filtered = malloc(n * sizeof(Particle));
    int count = 0;

    // Filter only particles that belong to the region.
    for (int i = 0; i < n; i++) {
        int region = get_region(p[i], spec);
        if (region != region_id) continue;
        filtered[count++] = p[i];
    }

    // Resize the array.
    filtered = realloc(filtered, count * sizeof(Particle));

    // Allocate to count pointer.
    *n_filtered = count;

    return filtered;
}
