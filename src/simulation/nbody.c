#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../utils/log.h"
#include "../utils/types.h"

#define SOFTENING_CONSTANT 10E-9F

/**
 * Updates the position of a particle for a given timestep.
 */
void update_position(Particle *particles, long long size, long double dt)
{
    LL_DEBUG("Updating position of %lld particles:", size);

    for (long long i = 0; i < size; i++) {
        particles[i].x += dt * particles[i].vx;
        particles[i].y += dt * particles[i].vy;
        LL_DEBUG("+ Particle %6.0lld: Velocity is (%0.9Lf, %0.9Lf); Displacing by (%0.9Lf, %0.9Lf); New position: (%0.9Lf, %0.9Lf)",
            i + 1, particles[i].vx, particles[i].vy, dt * particles[i].vx, dt * particles[i].vy, particles[i].x, particles[i].y);
    }
}

/**
 * Computes the new velocity for each particle for a given timestep.
 * Uses the gravitational force formula to compute the force, giving us the new velocity.
 */
void update_velocity(Particle *particles, long long size, long double dt)
{
    for (long long i = 0; i < size; i++) {
        long double fx = 0.0, fy = 0.0;

        LL_DEBUG("Computing force on particle %lld with dt = %0.6Lf:", i + 1, dt);

        // Compute the force of each particle on p0.
        for (long long j = 0; j < size; j++) {
            if (i == j) continue;

            LL_DEBUG("+ Particle %lld: ", j + 1);

            long double dx = particles[j].x - particles[i].x;
            long double dy = particles[j].y - particles[i].y;
            long double dist2 = dx * dx + dy * dy + SOFTENING_CONSTANT;
            long double dist = sqrtl(dist2);
            long double f = (particles[j].mass * particles[i].mass) / dist2;
            LL_DEBUG("  dx = %0.9Lf, dy = %0.9Lf, dist = %0.9Lf, dist2 = %0.9Lf, f = %0.9Lf", dx, dy, dist, dist2, f);

            assert(dist2 != 0.0f);

            fx += f * dx / dist;
            fy += f * dy / dist;
            LL_DEBUG("  fx += %0.9Lf = %0.9Lf, fy += %0.9Lf = %0.9Lf", f * dx / dist, fx, f * dy / dist, fy);

            assert(!isnan(fx) && !isnan(fy) && isfinite(fx) && isfinite(fy));
        }

        LL_DEBUG("+ Total force: %0.9Lf %0.9Lf", fx, fy);

        // Update the velocity.
        LL_DEBUG("+ Old velocity: vx = %0.9Lf, vy = %0.9Lf", particles[i].vx, particles[i].vy);
        particles[i].vx += dt * fx;
        particles[i].vy += dt * fy;
        LL_DEBUG("  New velocity: vx += %0.9Lf = %0.9Lf, vy += %0.9Lf = %0.9Lf", dt * fx, particles[i].vx, dt * fy, particles[i].vy);

        assert(!isnan(particles[i].vx) && !isnan(particles[i].vy) && isfinite(particles[i].vx) && isfinite(particles[i].vy));
    }
}
