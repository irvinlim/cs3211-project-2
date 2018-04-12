#ifndef TYPES_H
#define TYPES_H

/**
 * Enum for particle size.
 */
typedef enum particle_size_t {
    SMALL,
    LARGE,
} ParticleSize;

/**
 * Data structure for a large particle.
 */
typedef struct particle_t {
    // ID for the particle (for easier reference).
    int id;

    // Region for the particle (used during calculations).
    int region;

    // Type of particle (large or small).
    ParticleSize size;

    // Mass of the particle.
    long double mass;

    // Radius of the particle.
    long double radius;

    // Location coordinates
    long double x;
    long double y;

    // Velocity
    long double vx;
    long double vy;
} Particle;

/**
 * Data structure for the specification file.
 */
typedef struct spec_t {
    // Number of iterations to compute.
    int TimeSlots;

    // Unit time step for each iteration.
    long double TimeStep;

    // Max distance of adjacent regions to factor in.
    int Horizon;

    // Length of the 2-D grid for each region.
    int GridSize;

    // Number of small (gas) particles.
    int NumberOfSmallParticles;

    // Mass of small (gas) particles.
    long double SmallParticleMass;

    // Radius of small (gas) particles.
    long double SmallParticleRadius;

    // Number of large (gas) particles.
    int NumberOfLargeParticles;

    // Array of large particle data, in the following format:
    // 0: Mass; 1: Radius; 2: x-coordinate; 3: y-coordinate
    Particle *LargeParticles;

    // Length of the 2-D grid of all regions.
    // If there are 9 regions, PoolLength is 3.
    int PoolLength;

    // Total number of particles.
    int TotalNumberOfParticles;
} Spec;

#endif
