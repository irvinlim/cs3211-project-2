#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "log.h"
#include "multiproc.h"
#include "particles.h"

#define BITMAP_MAX 255

/**
 * Generates small particles in random starting locations,
 * within the specified boundaries.
 */
Particle *generate_small_particles(Spec spec, int n, int grid_size, long double start_x, long double start_y)
{
    // Initialize PRNG.
    time_t t;
    srand((unsigned)time(&t));

    // Allocate space for n particles.
    Particle *particles = malloc(sizeof(Particle) * n);

    // Generate small particles at random starting locations.
    for (int i = 0; i < n; i++) {
        particles[i] = (Particle){
            .id = i + spec.NumberOfLargeParticles,
            .region = -1, // actual region is found in the 2-D array key, this is only used for storing state during computation
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
Particle *generate_particles(Spec spec)
{
    // The small particles should be generated for a single region, not all regions.
    int grid_size = spec.GridSize;

    // Allocate space for all particles.
    Particle *particles = malloc(sizeof(Particle) * spec.TotalNumberOfParticles);

    // Copy large particles from spec.
    size_t large_particles_len = spec.NumberOfLargeParticles * sizeof(Particle);
    memcpy(particles, spec.LargeParticles, large_particles_len);

    // Generate small particles and allocate to the buffer.
    Particle *small_particles = generate_small_particles(spec, spec.NumberOfSmallParticles, grid_size, 0, 0);
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
    char delimiter = ',';

    // Get the maximum possible size of the string.
    int maxdigits = ceil(log10(n));
    int maxlen = maxdigits * n + n + 1;

    // Allocate a buffer large enough to put our string.
    char *buf = calloc(maxlen, sizeof(char));

    // Concatenate all IDs into a string.
    int currentlen = 0;
    for (int i = 0; i < n; i++) {
        int len = particles[i].id <= 1 ? 1 : ceil(log10(particles[i].id));

        // Add the ID to the buffer.
        sprintf(&buf[currentlen], "%d", particles[i].id);
        currentlen += len;

        // Add a delimiter.
        if (i < n - 1) buf[currentlen++] = delimiter;
    }

    if (currentlen == 0) {
        sprintf(buf, "None");
        currentlen += 4;
    }

    // Add null-terminating character.
    buf[currentlen++] = 0;

    LOG(level, "Process %d: %s - %s", get_process_id(), msg, buf);
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
