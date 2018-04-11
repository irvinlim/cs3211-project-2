#include "../utils/types.h"

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
Particle **update_position_and_region(long double dt, Spec spec, int num_regions, Particle **particles_by_region, int *sizes, int region_id);

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
void update_velocity(long double dt, Spec spec, int num_regions, Particle **particles_by_region, int *sizes, int region_id);
