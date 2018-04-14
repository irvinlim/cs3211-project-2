#include "../utils/types.h"

/**
 * Computes the new velocity for each particle for a given timestep, only for the given region ID.
 * Uses all other regions' particles to compute the force on the region's particles, in order to
 * compute the resultant velocity.
 * 
 * This method uses Newton's law of universal gravitation.
 * 
 * @param dt                    The time step value.
 * @param spec                  The program specification.
 * @param sizes                 Sizes of each array in particles_by_region.
 * @param particles_by_region   2-D array of particles, indexed by region ID.
 * @param num_regions           The number of regions.
 * @param region_id             The region whose particles' velocities should be updated.
 */
void update_velocity(long double dt, Spec spec, int *sizes, Particle **particles_by_region, int num_regions, int region_id);

/**
 * Updates the position and region of a particle for a given timestep.
 * 
 * @param dt                    The time step value.
 * @param spec                  The program specification.
 * @param sizes                 Sizes of each array in particles_by_region.
 * @param particles_by_region   2-D array of particles, indexed by region ID.
 * @param num_regions           The number of regions.
 * @param region_id             The region whose particles' positions should be updated.
 *                              Particles in all other regions will be discarded!
 *                              The updated particles will be placed in their respective regions after return.
 * @return                      Returns a new array of particles.
 */
Particle **update_position_and_region(long double dt, Spec spec, int *sizes, Particle **particles_by_region, int num_regions, int region_id);

/**
 * Updates the velocities of any particles in this process' region
 * if it is colliding with any other particle.
 * 
 * This method also counteracts any overlaps in particles by "pushing back"
 * the particles until they are just touching each other, so as to prevent
 * an infinite loop of collisions.
 * 
 * This computation assumes perfectly elastic collisions between particles
 * (i.e. momentum and kinetic energy is conserved).
 * 
 * @param spec                  The program specification.
 * @param sizes                 Sizes of each array in particles_by_region.
 * @param particles_by_region   2-D array of particles, indexed by region ID.
 * @param num_regions           The number of regions.
 * @param region_id             The region whose particles' velocities should be updated.
 */
void handle_collisions(Spec spec, int *sizes, Particle **particles_by_region, int num_regions, int region_id);

/**
 * Handle collisions against the walls of the pool area.
 * 
 * Assumes that the mass of the walls >> the mass of the particles,
 * which means that the velocity of the wall does not change 
 * (i.e. the wall velocity remains at zero).
 * 
 * In this case, we don't need to do elastic collision, we can just
 * invert the respective velocities.
 * 
 * @param spec          The program specification.
 * @param size          Size of the array.
 * @param particles     Array of particles whose velocities should be updated.
 */
void handle_wall_collisions(Spec spec, int size, Particle *particles);
