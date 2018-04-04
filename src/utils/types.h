#ifndef TYPES_H
#define TYPES_H

/**
 * Data structure for a large particle.
 */
typedef struct particle_t
{
    // Mass of the particle.
    long double mass;

    // Radius of the particle.
    long double radius;

    // Location x-coordinate.
    long double x;

    // Location y-coordinate.
    long double y;
} Particle;

/**
 * Data structure for the specification file.
 */
typedef struct spec_t
{
    // Number of iterations to compute.
    long TimeSlots;

    // Unit time step for each iteration.
    long double TimeStep;

    // Max distance of adjacent regions to factor in.
    long Horizon;

    // Length of the 2-D grid.
    long GridSize;

    // Number of small (gas) particles.
    long long NumberOfSmallParticles;

    // Mass of small (gas) particles.
    long double SmallParticleMass;

    // Radius of small (gas) particles.
    long double SmallParticleRadius;

    // Number of large (gas) particles.
    long long NumberOfLargeParticles;

    // Array of large particle data, in the following format:
    // 0: Mass; 1: Radius; 2: x-coordinate; 3: y-coordinate
    Particle *LargeParticles;
} Spec;

#endif
