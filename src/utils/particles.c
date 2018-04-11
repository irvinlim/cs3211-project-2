#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "log.h"
#include "multiproc.h"
#include "particles.h"

#define BITMAP_MAX 255

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
    print_particles(LOG_LEVEL_DEBUG, spec.TotalNumberOfParticles, particles);

    return particles;
}

/**
 * Generates a 2-D canvas of all particles.
 * 
 * Any values less than or equal to BITMAP_MAX represents
 * the presence of the body of a small particle, while
 * any value greater than BITMAP_MAX represents the body
 * of a large particle.
 * 
 * TODO: Handle 2-D particles array.
 */
int **generate_canvas(Spec spec, Particle *particles)
{
    // Get canvas length.
    int canvas_length = spec.GridSize * spec.PoolLength;

    // Allocate enough memory for a 2-D canvas.
    int **canvas = (int **)malloc(canvas_length * sizeof(int *));
    for (int i = 0; i < canvas_length; i++)
        canvas[i] = (int *)calloc(canvas_length, sizeof(int));

    // Iterate through all particles.
    for (int i = 0; i < spec.TotalNumberOfParticles; i++) {
        Particle p = particles[i];

        // Round up the coordinates of the particle's radius to find bounding box.
        int r = ceil(p.radius);

        // Iterate through all pixels occupied by the particle using simple
        // radius checking in the bounding box.
        for (int j = -r; j <= r; j++) {
            for (int k = -r; k <= r; k++) {
                if (j * j + k * k < p.radius * p.radius) {
                    // Get the coordinates relative to the origin.
                    int x = p.x + k;
                    int y = p.y + j;

                    // Prevent drawing outside of the bounds of the array.
                    if (x < 0 || x >= canvas_length || y < 0 || y >= canvas_length) continue;

                    // If the particle size is large, we immediately set the value to BITMAP_MAX + 1.
                    // Otherwise, we will increment the value, up to BITMAP_MAX.
                    if (p.size == LARGE || canvas[y][x] > BITMAP_MAX)
                        canvas[y][x] = BITMAP_MAX + 1;
                    else
                        canvas[y][x] = fmin(BITMAP_MAX, canvas[y][x] + 1);
                }
            }
        }
    }

    return canvas;
}

/**
 * Generate a heatmap of particles and saves it to an image file.
 * 
 * TODO: Handle 2-D particles array.
 */
void generate_heatmap(Spec spec, Particle *particles, char *outputfile)
{
    // Get canvas length.
    int canvas_length = spec.GridSize * spec.PoolLength;

    // Open file for writing.
    FILE *fp = fopen(outputfile, "w");
    if (fp == NULL) {
        LL_ERROR("Could not open %s for writing!", outputfile);
        exit(EXIT_FAILURE);
    }

    // Print PPM header.
    fprintf(fp, "P3\n%d %d\n%d\n", canvas_length, canvas_length, BITMAP_MAX);

    // Generate canvas from the list of particles.
    int **canvas = generate_canvas(spec, particles);

    // Print each cell.
    for (int y = 0; y < canvas_length; y++) {
        for (int x = 0; x < canvas_length; x++) {
            // If the value is greater than BITMAP_MAX, we draw a blue pixel.
            // Otherwise, we draw a red pixel whose intensity is the value.
            if (canvas[y][x] > BITMAP_MAX)
                fprintf(fp, "0 0 %d", BITMAP_MAX);
            else
                fprintf(fp, "%d 0 0", canvas[y][x]);

            if (x < canvas_length - 1)
                fprintf(fp, " ");
            else
                fprintf(fp, "\n");
        }
    }

    // Print success message.
    LL_SUCCESS("Successfully written image to %s.", outputfile);

    // Clean up.
    fclose(fp);
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
    if (level < log_level) return;

    // Map all IDs from Particles to print.
    int *ids = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) ids[i] = particles[i].id;
    print_ints(level, msg, n, ids);
    free(ids);
}

/**
 * Debug prints all particles.
 */
void print_particles(LogLevel level, int n, Particle *particles)
{
    if (n <= 0) return;

    int region_id = get_process_id();
    LOG(level, "Process %d: Dump of all %d particles: ", region_id, n);

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
