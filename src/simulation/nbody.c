#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../utils/log.h"
#include "../utils/types.h"
#include "regions.h"

#define SOFTENING_CONSTANT 10E-9F

/**
 * Updates the position of a particle for a given timestep.
 */
void update_position(Spec spec, Particle *p, long long n, long double dt, int region_id)
{
    LL_DEBUG("Updating position of %lld particles:", n);

    for (long long i = 0; i < n; i++) {
        // Skip if the particle is not in the specified region.
        if (region_id >= 0 && get_region(p[i], spec) != region_id) continue;

        // Compute the new position.
        p[i].x += dt * p[i].vx;
        p[i].y += dt * p[i].vy;

        LL_DEBUG("+ Particle %6.0lld: Velocity is (%0.9Lf, %0.9Lf); Displacing by (%0.9Lf, %0.9Lf); New position: (%0.9Lf, %0.9Lf)",
            i + 1, p[i].vx, p[i].vy, dt * p[i].vx, dt * p[i].vy, p[i].x, p[i].y);
    }
}

/**
 * Computes the new velocity for each particle for a given timestep.
 * Uses the gravitational force formula to compute the force, giving us the new velocity.
 */
void update_velocity(Spec spec, Particle *p, long long n, long double dt, int region_id, int horizon)
{
    for (long long i = 0; i < n; i++) {
        long double fx = 0.0, fy = 0.0;
        int pr = get_region(p[i], spec);

        // Skip if the particle is not in the specified region.
        if (region_id >= 0 && pr != region_id) continue;

        LL_DEBUG("Computing force on particle %lld with dt = %0.6Lf:", i + 1, dt);

        // Compute the force of each particle on p0.
        for (long long j = 0; j < n; j++) {
            // Don't compute the force of a particle on itself.
            if (i == j) continue;

            // Only compute the force of another particle on itself
            // if the regions are within the horizon threshold.
            int pr2 = get_region(p[i], spec);
            if (horizon >= 0 && get_horizon_dist(spec.PoolLength, pr, pr2) > horizon) continue;

            LL_DEBUG("+ Particle %lld: ", j + 1);

            long double dx = p[j].x - p[i].x;
            long double dy = p[j].y - p[i].y;
            long double dist2 = dx * dx + dy * dy + SOFTENING_CONSTANT;
            long double dist = sqrtl(dist2);
            long double f = (p[j].mass * p[i].mass) / dist2;

            LL_DEBUG("  dx = %0.9Lf, dy = %0.9Lf, dist = %0.9Lf, dist2 = %0.9Lf, f = %0.9Lf", dx, dy, dist, dist2, f);
            assert(dist2 != 0.0f);

            fx += f * dx / dist;
            fy += f * dy / dist;

            LL_DEBUG("  fx += %0.9Lf = %0.9Lf, fy += %0.9Lf = %0.9Lf", f * dx / dist, fx, f * dy / dist, fy);
            assert(!isnan(fx) && !isnan(fy) && isfinite(fx) && isfinite(fy));
        }

        LL_DEBUG("+ Total force: %0.9Lf %0.9Lf", fx, fy);

        // Update the velocity.
        LL_DEBUG("+ Old velocity: vx = %0.9Lf, vy = %0.9Lf", p[i].vx, p[i].vy);
        LL_DEBUG("  New velocity: vx += %0.9Lf = %0.9Lf, vy += %0.9Lf = %0.9Lf", dt * fx, p[i].vx + dt * fx, dt * fy, p[i].vy + dt * fy);
        p[i].vx += dt * fx;
        p[i].vy += dt * fy;

        assert(!isnan(p[i].vx) && !isnan(p[i].vy) && isfinite(p[i].vx) && isfinite(p[i].vy));
    }
}
