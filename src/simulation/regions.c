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
 * Allocates space for a 2-D array of arrays of particles, indexed by region.
 */
Particle **allocate_particles(int *sizes, int n_regions)
{
    Particle **particles = (Particle **)malloc(n_regions * sizeof(Particle *));
    for (int i = 0; i < n_regions; i++)
        particles[i] = (Particle *)malloc(sizes[i] * sizeof(Particle));

    return particles;
}

/**
 * Allocates space for as many particles as there are for each region.
 */
Particle **allocate_max_particles(int max_size, int n_regions)
{
    Particle **particles = (Particle **)malloc(n_regions * sizeof(Particle *));
    for (int i = 0; i < n_regions; i++)
        particles[i] = (Particle *)malloc(max_size * sizeof(Particle));

    return particles;
}

/**
 * Filters an array of particles by region.
 */
Particle *filter_by_regions(int *n_filtered, Spec spec, int *region_ids, int n_regions, Particle *p, int n)
{
    // Allocate space for all particles first.
    Particle *filtered = malloc(n * sizeof(Particle));
    int count = 0;

    // Filter only particles that belong to the region.
    for (int i = 0; i < n; i++) {
        int region = get_region(p[i], spec);
        for (int r = 0; r < n_regions; r++) {
            int region_id = region_ids[r];
            if (region != region_id) continue;
            filtered[count++] = p[i];
        }
    }

    // Resize the array.
    filtered = realloc(filtered, count * sizeof(Particle));

    // Allocate to count pointer.
    *n_filtered = count;

    return filtered;
}

/**
 * Filters an array of particles by region.
 */
Particle *filter_by_region(int *n_filtered, Spec spec, int region_id, Particle *p, int n)
{
    int regions[1];
    regions[0] = region_id;

    return filter_by_regions(n_filtered, spec, regions, 1, p, n);
}
