#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "particles.h"

#if defined(DEBUG) && DEBUG > 0
#define DEBUG_PRINT(x) printf x
#else // clang-format off
#define DEBUG_PRINT(x) do {} while (0);
#endif // clang-format on

/**
 * Generate all particles based on the provided specifications.
 */
Particle *generate_particles(Spec spec)
{
    long long i;
    time_t t;

    // Initialize PRNG.
    srand((unsigned)time(&t));

    // Total number of particles is large + small.
    long long total_particles = spec.NumberOfLargeParticles + spec.NumberOfSmallParticles;

    // Generate particles.
    Particle *particles = malloc(sizeof(Particle) * total_particles);

    // Copy large particles from spec.
    memcpy(particles, spec.LargeParticles, spec.NumberOfLargeParticles * sizeof(Particle));

    // Generate small particles at random starting locations.
    for (i = spec.NumberOfLargeParticles; i < total_particles; i++)
        particles[i] = (Particle){
            .size = SMALL,
            .mass = spec.SmallParticleMass,
            .radius = spec.SmallParticleRadius,
            .x = rand() % spec.GridSize,
            .y = rand() % spec.GridSize,
        };

    // Debug print all particles.
    print_particles(particles);

    return particles;
}

void print_particles(Particle *particles)
{
    int i;

    DEBUG_PRINT(("\033[0;36m"));
    DEBUG_PRINT(("Generated particles:\n"));
    for (i = 0; i < sizeof(particles); i++)
        print_particle(particles[i]);
    DEBUG_PRINT(("\033[0m\n"));
}

/**
 * Prints details about the particle.
 */
void print_particle(Particle particle)
{
    DEBUG_PRINT((
        "+ Size: %d; Mass: %Lf; Radius: %Lf; Position: (%Lf, %Lf)\n",
        particle.size,
        particle.mass,
        particle.radius,
        particle.x,
        particle.y));
}
