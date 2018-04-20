#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/common.h"
#include "../utils/log.h"
#include "../utils/particles.h"
#include "../utils/regions.h"
#include "../utils/types.h"
#include "../utils/vector.h"

#define SOFTENING_PARAM 0.0001F

void update_position(long double dt, Spec spec, int size, Particle *particles, int region_id)
{
    LL_DEBUG("Updating positions of %d particles in region %d:", size, region_id);

    // Update the positions of particles in the specified region.
    for (int i = 0; i < size; i++) {
        Particle p = particles[i];

        LL_DEBUG("+ Particle %6.0d: ", p.id);
        // Denormalize the position wrt region first.
        particles[i].x = denorm_region_x(p.x, region_id, spec);
        particles[i].y = denorm_region_y(p.y, region_id, spec);
        p = particles[i];

        // Compute the new position.
        particles[i].x += dt * p.vx;
        particles[i].y += dt * p.vy;
        p = particles[i];

        // Wrap the particle around all regions if necessary.
        particles[i].x = wrap_around(p.x, spec.GridSize * spec.PoolLength);
        particles[i].y = wrap_around(p.y, spec.GridSize * spec.PoolLength);
        p = particles[i];

        // Calculate the region of the new position.
        int region = get_denorm_particle_region(p, spec);
        LL_DEBUG("  Region = %d", region);

        // Update the region field here, so that we can sort it later.
        particles[i].region = region;
        p = particles[i];

        // Re-normalize the position wrt region.
        particles[i].x = norm_region(p.x, spec);
        particles[i].y = norm_region(p.y, spec);
        p = particles[i];

        LL_DEBUG2("  Velocity   = (%0.9Lf, %0.9Lf), Displacement = (%0.9Lf, %0.9Lf)", p.vx, p.vy, dt * p.vx, dt * p.vy);
        LL_DEBUG("  New (x, y) = (%0.9Lf, %0.9Lf)", p.x, p.y);

        // Perform assertions to aid debugging.
        if (isnan(p.x) || isnan(p.y) || !isfinite(p.x) || !isfinite(p.y)) {
            LL_ERROR("Assertion failed: (x,y) = (%0.9Lf, %0.9Lf); (vx, vy) = (%0.9Lf, %0.9Lf)", p.x, p.y, p.vx, p.vy);
            assert(!isnan(p.x) && !isnan(p.y) && isfinite(p.x) && isfinite(p.y));
        }
    }
}

Particle **reallocate_for_region(Spec spec, int *sizes, int num_particles, Particle *particles, int num_regions)
{
    // Truncate the sizes of all regions first.
    for (int i = 0; i < num_regions; i++) sizes[i] = 0;

    // Compute the number of particles in their resultant regions.
    for (int i = 0; i < num_particles; i++) sizes[particles[i].region]++;

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
    for (int i = 0; i < num_particles; i++) {
        // Extract region from region field that we populated earlier.
        int region = particles[i].region;

        // Append the particle into the array.
        memcpy(&new_particles[region][counters[region]], &particles[i], sizeof(Particle));

        // Increment the counter.
        counters[region]++;
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

    return new_particles;
}

/**
 * Computes the new velocity for each particle for a given timestep, only for the given region ID.
 * Uses all other regions' particles to compute the force on the region's particles, in order to
 * compute the resultant velocity.
 * 
 * This method uses Newton's law of universal gravitation.
 */
void update_velocity(long double dt, Spec spec, int *sizes, Particle **particles_by_region, int num_regions, int region_id)
{
    // Iterate through all particles in the given region.
    for (int i = 0; i < sizes[region_id]; i++) {
        long double fx = 0.0, fy = 0.0;

        Particle p0 = particles_by_region[region_id][i];
        LL_DEBUG("Computing force on region %d, particle %d with dt = %0.6Lf:", region_id, p0.id, dt);
        LL_DEBUG2("  mass            = %0.9Lf", p0.mass);
        LL_DEBUG("+ p0(x, y)        = (%0.9Lf, %0.9Lf)", p0.x, p0.y);
        LL_DEBUG("  denorm_p0(x, y) = (%0.9Lf, %0.9Lf)", denorm_region_x(p0.x, region_id, spec), denorm_region_y(p0.y, region_id, spec));

        // Compute the force of each particle in all regions on p0.
        for (int region = 0; region < num_regions; region++) {
            for (int j = 0; j < sizes[region]; j++) {
                // Don't compute the force of a particle on itself.
                if (region == region_id && i == j) continue;

                Particle p1 = particles_by_region[region][j];
                LL_DEBUG2("+ Region %d, particle %d: ", region, p1.id);
                LL_DEBUG2("    mass            = %0.9Lf", p1.mass);
                LL_DEBUG2("    p1(x, y)        = (%0.9Lf, %0.9Lf)", p1.x, p1.y);
                LL_DEBUG2("    denorm_p1(x, y) = (%0.9Lf, %0.9Lf)", denorm_region_x(p1.x, region, spec), denorm_region_y(p1.y, region, spec));

                long double dx = denorm_region_x(p1.x, region, spec) - denorm_region_x(p0.x, region_id, spec);
                long double dy = denorm_region_y(p1.y, region, spec) - denorm_region_y(p0.y, region_id, spec);
                long double dist2 = dx * dx + dy * dy + SOFTENING_PARAM * SOFTENING_PARAM;
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

/**
 * Updates the velocities of any particles in this process' region
 * if it is colliding with any other particle.
 */
void handle_collisions(Spec spec, int *sizes, Particle **particles_by_region, int num_regions, int region_id)
{
    // Count the number of collisions we handled in total.
    int total_collisions = 0;

    // Iterate each particle in the region.
    for (int i = 0; i < sizes[region_id]; i++) {
        Particle p1 = particles_by_region[region_id][i];

        Vector pos1 = { .x = denorm_region_x(p1.x, region_id, spec), .y = denorm_region_y(p1.y, region_id, spec) };
        Vector vel1 = { .x = p1.vx, .y = p1.vy };
        LL_DEBUG("Handling collisions for region %d, particle %d:", region_id, p1.id);
        LL_DEBUG2("  (x, y)     = (%0.9Lf, %0.9Lf)", pos1.x, pos1.y);
        LL_DEBUG2("  (vx, vy)   = (%0.9Lf, %0.9Lf)", vel1.x, vel1.y);
        LL_DEBUG2("  m, r       = (%0.9Lf, %0.9Lf)", p1.mass, p1.radius);

        // Check for collisions against all other particles in every region.
        for (int region = 0; region < num_regions; region++) {
            for (int j = 0; j < sizes[region]; j++) {
                // Don't collide with yourself; don't double-count
                // collisions of two particles in the same region.
                // Only smaller-indexed particles should handle collisions with
                // larger-indexed particles; both sides will be updated.
                if (region == region_id && i >= j) continue;

                Particle p2 = particles_by_region[region][j];

                Vector pos2 = { .x = denorm_region_x(p2.x, region, spec), .y = denorm_region_y(p2.y, region, spec) };
                Vector vel2 = { .x = p2.vx, .y = p2.vy };
                LL_DEBUG2("+ Region %d, particle %d: ", region, p2.id);
                LL_DEBUG2("    (x, y)   = (%0.9Lf, %0.9Lf)", pos2.x, pos2.y);
                LL_DEBUG2("    (vx, vy) = (%0.9Lf, %0.9Lf)", vel2.x, vel2.y);
                LL_DEBUG2("    m, r     = (%0.9Lf, %0.9Lf)", p2.mass, p2.radius);

                Vector vel_diff = vec_sub(vel1, vel2);
                Vector pos_diff = vec_sub(pos1, pos2);
                long double dist = vec_len(pos_diff);
                long double distSq = dist * dist;
                long double r_sum = p1.radius + p2.radius;

                LL_DEBUG2("    dist     = %0.9Lf", dist);
                LL_DEBUG2("    r1 + r2  = %0.9Lf", r_sum);

                // Check if particles are collided.
                if (dist > r_sum) continue;
                LL_DEBUG("  + Collision detected between (%d, %d) and (%d, %d)!", region_id, p1.id, region, p2.id);
                total_collisions++;

                // Handle the case when the particles overlap, AND their unit vectors are equal.
                // Add an arbitrary constant to both x and y velocities so that they are different.
                if (vec_len(pos_diff) == 0 && vec_normalize(vel1).x == vec_normalize(vel2).x && vec_normalize(vel1).y == vec_normalize(vel2).y) {
                    vel1 = (Vector){ .x = p1.vx - SOFTENING_PARAM, .y = p1.vy + SOFTENING_PARAM };
                    vel2 = (Vector){ .x = p2.vx + SOFTENING_PARAM, .y = p2.vy - SOFTENING_PARAM };
                    LL_DEBUG2("      Overlap! Adding arbitrary constant to p1.vel = (%0.9Lf, %0.9Lf); p2.vel = (%0.9Lf, %0.9Lf)", vel1.x, vel1.y, vel2.x, vel2.y);
                    vel_diff = vec_sub(vel1, vel2);
                }

                // Move the particles backwards till the point that they are just touching each other.
                long double back_len = (r_sum - dist) / 2;
                Vector b1 = vec_mul_scalar(-back_len, vec_normalize(vel1));
                Vector b2 = vec_mul_scalar(-back_len, vec_normalize(vel2));
                LL_DEBUG2("      back   = %0.9Lf", back_len);
                LL_DEBUG2("      b1     = (%0.9Lf, %0.9Lf)", b1.x, b1.y);
                LL_DEBUG2("      b2     = (%0.9Lf, %0.9Lf)", b2.x, b2.y);

                particles_by_region[region_id][i].x += b1.x;
                particles_by_region[region_id][i].y += b1.y;
                particles_by_region[region][j].x += b2.x;
                particles_by_region[region][j].y += b2.y;

                // Recalculate positions and distance since it may have been updated.
                p1 = particles_by_region[region_id][i];
                p2 = particles_by_region[region][j];
                pos1 = (Vector){ .x = denorm_region_x(p1.x, region_id, spec), .y = denorm_region_y(p1.y, region_id, spec) };
                pos2 = (Vector){ .x = denorm_region_x(p2.x, region, spec), .y = denorm_region_y(p2.y, region, spec) };
                pos_diff = vec_sub(pos1, pos2);
                dist = vec_len(pos_diff);
                distSq = dist * dist;

                // Update the velocities for p1, assuming perfectly elastic collision.
                long double mass_term1 = (2 * p2.mass) / (p1.mass + p2.mass);
                long double dot_term1 = vec_dot(vel_diff, pos_diff) / distSq;
                Vector sub_v1 = vec_mul_scalar(mass_term1 * dot_term1, pos_diff);
                particles_by_region[region_id][i].vx -= sub_v1.x;
                particles_by_region[region_id][i].vy -= sub_v1.y;
                LL_DEBUG2("      v1     - (%0.9Lf, %0.9Lf) = (%0.9Lf, %0.9Lf)", sub_v1.x, sub_v1.y, p1.vx - sub_v1.x, p1.vy - sub_v1.y);

                // Do the same for p2.
                long double mass_term2 = (2 * p1.mass) / (p1.mass + p2.mass);
                long double dot_term2 = vec_dot(vec_sub(vel2, vel1), vec_sub(pos2, pos1)) / distSq;
                Vector sub_v2 = vec_mul_scalar(mass_term2 * dot_term2, vec_sub(pos2, pos1));
                particles_by_region[region][j].vx -= sub_v2.x;
                particles_by_region[region][j].vy -= sub_v2.y;
                LL_DEBUG2("      v2     - (%0.9Lf, %0.9Lf) = (%0.9Lf, %0.9Lf)", sub_v2.x, sub_v2.y, p2.vx - sub_v2.x, p2.vy - sub_v2.y);

                p1 = particles_by_region[region_id][i];
                p2 = particles_by_region[region][j];

                // Perform assertions to aid debugging.
                if (isnan(p1.x) || isnan(p1.y) || !isfinite(p1.x) || !isfinite(p1.y)
                    || isnan(p2.x) || isnan(p2.y) || !isfinite(p2.x) || !isfinite(p2.y)
                    || isnan(p1.vx) || isnan(p1.vy) || !isfinite(p1.vx) || !isfinite(p1.vy)
                    || isnan(p2.vx) || isnan(p2.vy) || !isfinite(p2.vx) || !isfinite(p2.vy)) {
                    LL_ERROR("%s", "Assertion failed in handle_collisions.");
                    LL_ERROR("p1: (x,y) = (%0.9Lf, %0.9Lf); (vx, vy) = (%0.9Lf, %0.9Lf)", p1.x, p1.y, p1.vx, p1.vy);
                    LL_ERROR("p2: (x,y) = (%0.9Lf, %0.9Lf); (vx, vy) = (%0.9Lf, %0.9Lf)", p2.x, p2.y, p2.vx, p2.vy);

                    assert(!isnan(p1.x) && !isnan(p1.y) && isfinite(p1.x) && isfinite(p1.y));
                    assert(!isnan(p2.x) && !isnan(p2.y) && isfinite(p2.x) && isfinite(p2.y));
                    assert(!isnan(p1.vx) && !isnan(p1.vy) && isfinite(p1.vx) && isfinite(p1.vy));
                    assert(!isnan(p2.vx) && !isnan(p2.vy) && isfinite(p2.vx) && isfinite(p2.vy));
                }
            }
        }
    }

    LL_VERBOSE2("Total number of particle collisions for region %d: %d", region_id, total_collisions);
}

/**
 * Handle collisions against the walls of the pool area.
 */
void handle_wall_collisions(Spec spec, int size, Particle *particles, int region_id)
{
    // Count the number of collisions we handled in total.
    int total_collisions = 0;

    for (int i = 0; i < size; i++) {
        Particle p = particles[i];

        Vector pos = { .x = denorm_region_x(p.x, region_id, spec), .y = denorm_region_y(p.y, region_id, spec) };
        LL_DEBUG("Handling wall collisions for region %d, particle %d:", region_id, p.id);
        LL_DEBUG2("  (x, y)     = (%0.9Lf, %0.9Lf)", pos.x, pos.y);

        long double dist_top = pos.y;
        long double dist_bot = spec.PoolLength * spec.GridSize - pos.y;
        long double dist_lft = pos.x;
        long double dist_rgt = spec.PoolLength * spec.GridSize - pos.x;

        // Handle top wall collisions.
        if (dist_top < p.radius) {
            LL_DEBUG2("  + Collision with top wall: p.y += %0.9Lf", p.radius - dist_top);
            particles[i].y += p.radius - dist_top;
            particles[i].vy *= -1;
            total_collisions++;
        }

        // Handle bottom wall collisions.
        if (dist_bot < p.radius) {
            LL_DEBUG2("  + Collision with bottom wall: p.y -= %0.9Lf", p.radius - dist_bot);
            particles[i].y -= p.radius - dist_bot;
            particles[i].vy *= -1;
            total_collisions++;
        }

        // Handle left wall collisions.
        if (dist_lft < p.radius) {
            LL_DEBUG2("  + Collision with left wall: p.x += %0.9Lf", p.radius - dist_lft);
            particles[i].x += p.radius - dist_lft;
            particles[i].vx *= -1;
            total_collisions++;
        }

        // Handle right wall collisions.
        if (dist_rgt < p.radius) {
            LL_DEBUG2("  + Collision with right wall: p.x -= %0.9Lf", p.radius - dist_rgt);
            particles[i].x -= p.radius - dist_rgt;
            particles[i].vx *= -1;
            total_collisions++;
        }

        // Perform assertions to aid debugging.
        p = particles[i];
        assert(!isnan(p.x) && !isnan(p.y) && isfinite(p.x) && isfinite(p.y));
        assert(!isnan(p.vx) && !isnan(p.vy) && isfinite(p.vx) && isfinite(p.vy));
    }

    LL_VERBOSE2("Total number of wall collisions for region %d: %d", region_id, total_collisions);
}
