#include "../utils/types.h"

/**
 * Updates the position of all particles for a given timestep.
 * 
 * @param spec          The program specification.
 * @param p             Array of particles to compute the new position for.
 * @param n             The size of p.
 * @param dt            The time step value.
 * @param region_id     Only performs computation if the particle resides in the particular region.
 *                      To compute for all regions, pass -1 for region_id.
 */
void update_position(Spec spec, Particle *p, long long n, long double dt, int region_id);

/**
 * Computes the new velocity for each particle for a given timestep.
 * Uses the gravitational force formula to compute the force, giving us the new velocity.
 * 
 * @param spec          The program specification.
 * @param p             Array of particles to compute the new velocity for, as well as against.
 * @param n             The size of p.
 * @param dt            The time step value.
 * @param region_id     Only performs computation if the particle resides in the particular region.
 *                      To compute for all regions, pass -1 for region_id.
 * @param horizon       Only computes the force by particles within this horizon distance threshold.
 *                      To compute the forces applied by particles in all regions, pass -1 for horizon.
 */
void update_velocity(Spec spec, Particle *p, long long n, long double dt, int region_id, int horizon);
