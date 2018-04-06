#include "../utils/types.h"

/**
 * Updates the position of all particles for a given timestep.
 */
void update_position(Particle *particles, long long size, long double dt);

/**
 * Computes the new velocity for each particle for a given timestep.
 * Uses the gravitational force formula to compute the force, giving us the new velocity.
 */
void update_velocity(Particle *particles, long long size, long double dt);
