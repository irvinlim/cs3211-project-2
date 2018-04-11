#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/common.h"
#include "../utils/log.h"
#include "../utils/particles.h"
#include "../utils/types.h"
#include "regions.h"

#define SOFTENING_CONSTANT 10E-9F

/**
 * Updates the position and region of a particle for a given timestep.
 * 
 * @param dt                    The time step value.
 * @param spec                  The program specification.
 * @param num_regions           The number of regions.
 * @param particles_by_region   2-D array of particles, indexed by region ID.
 * @param sizes                 Sizes of each array in particles_by_region.
 * @param region_id             The region whose particles' positions should be updated.
 *                              Particles in all other regions will be discarded!
 *                              The updated particles will be placed in their respective regions after return.
 * @return                      Returns a new array of particles.
 */
Particle **update_position_and_region(long double dt, Spec spec, int num_regions, Particle **particles_by_region, int *sizes, int region_id)
{
    // First we store the number of particles that we had computed (and wish to update positions for).
    int n = sizes[region_id];
    LL_DEBUG("Updating positions and regions of %d particles in region %d:", n, region_id);

    // Truncate the sizes of all regions first.
    for (int i = 0; i < num_regions; i++) sizes[i] = 0;

    // Update the positions of particles in the specified region and tabulate the size.
    for (int i = 0; i < n; i++) {
        LL_DEBUG("+ Particle %6.0d: ", i + 1);
        // Denormalize the position wrt region first.
        particles_by_region[region_id][i].x = denorm_region_x(particles_by_region[region_id][i].x, region_id, spec);
        particles_by_region[region_id][i].y = denorm_region_y(particles_by_region[region_id][i].y, region_id, spec);

        // Compute the new position.
        particles_by_region[region_id][i].x += dt * particles_by_region[region_id][i].vx;
        particles_by_region[region_id][i].y += dt * particles_by_region[region_id][i].vy;

        // Wrap the particle around all regions if necessary.
        particles_by_region[region_id][i].x = wrap_around(particles_by_region[region_id][i].x, spec.GridSize * spec.PoolLength);
        particles_by_region[region_id][i].y = wrap_around(particles_by_region[region_id][i].y, spec.GridSize * spec.PoolLength);

        // Calculate the region of the new position.
        int region = get_region(particles_by_region[region_id][i], spec);
        LL_DEBUG("  Region = %d", region);
        sizes[region]++;

        // Update the region field here, so that we can sort it later.
        particles_by_region[region_id][i].region = region;

        // Re-normalize the position wrt region.
        particles_by_region[region_id][i].x = norm_region(particles_by_region[region_id][i].x, spec);
        particles_by_region[region_id][i].y = norm_region(particles_by_region[region_id][i].y, spec);

        Particle p = particles_by_region[region_id][i];
        LL_DEBUG2("  Velocity   = (%0.9Lf, %0.9Lf), Displacement = (%0.9Lf, %0.9Lf)", p.vx, p.vy, dt * p.vx, dt * p.vy);
        LL_DEBUG("  New (x, y) = (%0.9Lf, %0.9Lf)", p.x, p.y);
        assert(!isnan(p.x) && !isnan(p.y) && isfinite(p.x) && isfinite(p.y));
    }

    // Debug print the final sizes.
    print_ints(LOG_LEVEL_DEBUG2, "Resultant sizes after updating positions", num_regions, sizes);

    // Keep track of number of particles stored within each region.
    int *counters = calloc(num_regions, sizeof(int));
    assert(counters != NULL);

    // Allocate the sizes for a new particles array according to the sizes computed.
    Particle **new_particles = allocate_particles(sizes, num_regions);
    assert(new_particles != NULL);

    // Copy the particles into the new array.
    LL_DEBUG2("%s", "Separating particles from original 2-D array into their own regions...");
    for (int i = 0; i < n; i++) {
        // Extract region from region field that we populated earlier.
        int region = particles_by_region[region_id][i].region;

        // Append the particle into the array.
        memcpy(&new_particles[region][counters[region]], &particles_by_region[region_id][i], sizeof(Particle));

        // Increment the counter.
        counters[region] += 1;
        assert(counters[region] > 0 && counters[region] <= spec.TotalNumberOfParticles * num_regions);
    }

    // Ensure that the sizes for counters and tabulated sizes are equal.
    for (int i = 0; i < num_regions; i++) assert(counters[i] == sizes[i]);

    // Debug particles that were copied into the new array.
    char msg[50];
    for (int i = 0; i < num_regions; i++) {
        sprintf(msg, "Dump of particles for region %d", i);
        print_particles(LOG_LEVEL_DEBUG2, msg, sizes[i], new_particles[i]);
    }

    // Free buffers.
    free(counters);
    free(particles_by_region);

    return new_particles;
}

/**
 * Computes the new velocity for each particle for a given timestep, only for the given region ID.
 * Uses all other regions' particles to compute the force on the region's particles, in order to
 * compute the resultant velocity.
 * 
 * This method uses Newton's law of universal gravitation.
 * 
 * @param dt                    The time step value.
 * @param spec                  The program specification.
 * @param num_regions           The number of regions.
 * @param particles_by_region   2-D array of particles, indexed by region ID.
 * @param sizes                 Sizes of each array in particles_by_region.
 * @param region_id             The region whose particles' velocities should be updated.
 *                              Particles in all other regions passed in through particles_by_region 
 *                              will be used in the computation of the resultant velocity.
 */
void update_velocity(long double dt, Spec spec, int num_regions, Particle **particles_by_region, int *sizes, int region_id)
{
    // Iterate through all particles in the given region.
    for (int i = 0; i < sizes[region_id]; i++) {
        long double fx = 0.0, fy = 0.0;

        Particle p0 = particles_by_region[region_id][i];
        LL_DEBUG("Computing force on particle %d with dt = %0.6Lf:", i + 1, dt);
        LL_DEBUG("+ p0(x, y)        = (%0.9Lf, %0.9Lf)", p0.x, p0.y);
        LL_DEBUG("  denorm_p0(x, y) = (%0.9Lf, %0.9Lf)", denorm_region_x(p0.x, region_id, spec), denorm_region_y(p0.y, region_id, spec));

        // Compute the force of each particle in all regions on p0.
        for (int region = 0; region < num_regions; region++) {
            for (int j = 0; j < sizes[region]; j++) {
                // Don't compute the force of a particle on itself.
                if (region == region_id && i == j) continue;

                Particle p1 = particles_by_region[region][j];
                LL_DEBUG2("+ Region %d, particle %d: ", region, j + 1);
                LL_DEBUG2("    p1(x, y)        = (%0.9Lf, %0.9Lf)", p1.x, p1.y);
                LL_DEBUG2("    denorm_p1(x, y) = (%0.9Lf, %0.9Lf)", denorm_region_x(p1.x, region, spec), denorm_region_y(p1.y, region, spec));

                long double dx = denorm_region_x(p1.x, region, spec) - denorm_region_x(p0.x, region_id, spec);
                long double dy = denorm_region_y(p1.y, region, spec) - denorm_region_y(p0.y, region_id, spec);
                long double dist2 = dx * dx + dy * dy + SOFTENING_CONSTANT;
                long double dist = sqrtl(dist2);
                long double f = (p1.mass * p0.mass) / dist2;

                LL_DEBUG2("    dx = %0.9Lf, dy = %0.9Lf, dist = %0.9Lf, dist2 = %0.9Lf, f = %0.9Lf", dx, dy, dist, dist2, f);
                assert(dist2 != 0.0f);

                fx += f * dx / dist;
                fy += f * dy / dist;

                LL_DEBUG2("    fx += %0.9Lf = %0.9Lf, fy += %0.9Lf = %0.9Lf", f * dx / dist, fx, f * dy / dist, fy);
                assert(!isnan(fx) && !isnan(fy) && isfinite(fx) && isfinite(fy));
            }
        }

        LL_DEBUG2("  Total force: %0.9Lf %0.9Lf", fx, fy);

        // Update the velocity.
        LL_DEBUG2("  Old (vx, vy)    = (%0.9Lf, %0.9Lf)", p0.vx, p0.vy);
        LL_DEBUG("  New (vx, vy)    = (%0.9Lf, %0.9Lf)", p0.vx + dt * fx, p0.vy + dt * fy);
        particles_by_region[region_id][i].vx += dt * fx;
        particles_by_region[region_id][i].vy += dt * fy;

        assert(!isnan(particles_by_region[region_id][i].vx)
            && !isnan(particles_by_region[region_id][i].vy)
            && isfinite(particles_by_region[region_id][i].vx)
            && isfinite(particles_by_region[region_id][i].vy));
    }
}
