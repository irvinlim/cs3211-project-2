#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "log.h"
#include "multiproc.h"
#include "particles.h"

/**
 * Allocates space for a 2-D array of arrays of particles, indexed by region.
 */
Particle **allocate_particles(int *sizes, int n_regions)
{
    Particle **particles = (Particle **)malloc(n_regions * sizeof(Particle *));
    for (int i = 0; i < n_regions; i++)
        particles[i] = (Particle *)malloc(sizes[i] * sizeof(Particle));

    return particles;
}

/**
 * Deallocate space that was reserved for a 2-D array of particles.
 */
void deallocate_particles(Particle **particles, int n_regions)
{
    for (int i = 0; i < n_regions; i++) free(particles[i]);
    free(particles);
}

/**
 * Generates small particles in random starting locations,
 * within the specified boundaries.
 */
Particle *generate_small_particles(int region_id, Spec spec, int n, int grid_size, long double start_x, long double start_y)
{
    // Initialize PRNG.
    time_t t;
    srand((unsigned)time(&t));

    // Allocate space for n particles.
    Particle *particles = malloc(sizeof(Particle) * n);

    // Generate small particles at random starting locations.
    for (int i = 0; i < n; i++) {
        particles[i] = (Particle){
            .id = region_id * spec.TotalNumberOfParticles + spec.NumberOfLargeParticles + i,
            .region = region_id,
            .size = SMALL,
            .mass = spec.SmallParticleMass,
            .radius = spec.SmallParticleRadius,
            .x = start_x + rand() % grid_size,
            .y = start_y + rand() % grid_size,
            .vx = 0.0L,
            .vy = 0.0L,
        };
    }

    return particles;
}

/**
 * Generate both small and large particles for a single region, according to the given spec.
 * The positions of the small particles will be randomized anywhere within the region.
 */
Particle *generate_particles(int region_id, Spec spec)
{
    // The small particles should be generated for a single region, not all regions.
    int grid_size = spec.GridSize;

    // Allocate space for all particles.
    Particle *particles = malloc(sizeof(Particle) * spec.TotalNumberOfParticles);

    // Copy large particles from spec.
    size_t large_particles_len = spec.NumberOfLargeParticles * sizeof(Particle);
    memcpy(particles, spec.LargeParticles, large_particles_len);

    // Generate small particles and allocate to the buffer.
    Particle *small_particles = generate_small_particles(region_id, spec, spec.NumberOfSmallParticles, grid_size, 0, 0);
    size_t small_particles_len = spec.NumberOfSmallParticles * sizeof(Particle);
    memcpy(&particles[spec.NumberOfLargeParticles], small_particles, small_particles_len);

    // Debug print all particles.
    print_particles(LOG_LEVEL_DEBUG, "Generated particles", spec.TotalNumberOfParticles, particles);

    return particles;
}

/**
 * Debug prints details about a particle.
 */
void print_particle(LogLevel level, Particle particle)
{
    LOG(level,
        "- ID: %d, Size: %d; Mass: %0.2Lf; Radius: %0.2Lf; Position: (%0.2Lf, %0.2Lf)",
        particle.id,
        particle.size,
        particle.mass,
        particle.radius,
        particle.x,
        particle.y);
}

/**
 * Prints a concatenated list of all particle IDs.
 */
void print_particle_ids(LogLevel level, char *msg, int n, Particle *particles)
{
    // Don't do anything if log_level is lesser than level (optimisation).
    if (level > log_level) return;

    // Map all IDs from Particles to print.
    int *ids = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) ids[i] = particles[i].id;
    print_ints(level, msg, n, ids);
    free(ids);
}

/**
 * Debug prints all particles.
 */
void print_particles(LogLevel level, char *msg, int n, Particle *particles)
{
    if (n <= 0) return;

    LOG(level, "%s (%d particles): ", msg, n);

    for (int i = 0; i < n; i++)
        LOG(level,
            "+ ID: %d, Size: %d; Mass: %Lf; Radius: %Lf; Position: (%Lf, %Lf); Velocity: (%Lf, %Lf)",
            particles[i].id,
            particles[i].size,
            particles[i].mass,
            particles[i].radius,
            particles[i].x,
            particles[i].y,
            particles[i].vx,
            particles[i].vy);
}
