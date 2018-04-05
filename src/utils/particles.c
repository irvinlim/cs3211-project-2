#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "particles.h"
#include "log.h"

#define BITMAP_MAX 255

/**
 * Generate all particles based on the provided specifications.
 */
Particle *generate_particles(Spec spec)
{
    long long i;
    time_t t;

    // Initialize PRNG.
    srand((unsigned)time(&t));

    // Generate particles.
    Particle *particles = malloc(sizeof(Particle) * spec.TotalNumberOfParticles);

    // Copy large particles from spec.
    memcpy(particles, spec.LargeParticles, spec.NumberOfLargeParticles * sizeof(Particle));

    // Generate small particles at random starting locations.
    for (i = spec.NumberOfLargeParticles; i < spec.TotalNumberOfParticles; i++)
        particles[i] = (Particle){
            .size = SMALL,
            .mass = spec.SmallParticleMass,
            .radius = spec.SmallParticleRadius,
            .x = rand() % spec.GridSize,
            .y = rand() % spec.GridSize,
        };

    // Debug print all particles.
    print_particles(spec.TotalNumberOfParticles, particles);

    return particles;
}

/**
 * Generates a 2-D canvas of all particles.
 * 
 * Any values less than or equal to BITMAP_MAX represents
 * the presence of the body of a small particle, while
 * any value greater than BITMAP_MAX represents the body
 * of a large particle.
 */
int **generate_canvas(Spec spec, Particle *particles)
{
    // Get canvas length.
    long canvas_length = spec.GridSize * spec.PoolLength;

    // Allocate enough memory for a 2-D canvas.
    int **canvas = (int **)malloc(canvas_length * sizeof(int *));
    for (long i = 0; i < canvas_length; i++)
        canvas[i] = (int *)calloc(canvas_length, sizeof(int));

    // Iterate through all particles.
    for (long i = 0; i < spec.TotalNumberOfParticles; i++)
    {
        Particle p = particles[i];

        // Round down the coordinates of the particle's origin and radius.
        long x0 = floor(p.x);
        long y0 = floor(p.y);
        long r = floor(p.radius);

        // Iterate through all pixels occupied by the particle using simple
        // radius checking in the bounding box.
        for (long j = -r; j <= r; j++)
        {
            for (long k = -r; k <= r; k++)
            {
                if (j * j + k * k <= r * r)
                {
                    // Get the coordinates relative to the origin.
                    long x = x0 + k;
                    long y = y0 + j;

                    // Prevent drawing outside of the bounds of the array.
                    if (x < 0 || x >= canvas_length || y < 0 || y >= canvas_length)
                        continue;

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
 */
void generate_heatmap(Spec spec, Particle *particles, char *outputfile)
{
    // Get canvas length.
    long canvas_length = spec.GridSize * spec.PoolLength;

    // Open file for writing.
    FILE *fp = fopen(outputfile, "w");
    if (fp == NULL)
    {
        LL_ERROR("Could not open %s for writing!", outputfile);
        exit(EXIT_FAILURE);
    }

    // Print PPM header.
    fprintf(fp, "P3\n%ld %ld\n%d\n", spec.GridSize, spec.GridSize, BITMAP_MAX);

    // Generate canvas from the list of particles.
    int **canvas = generate_canvas(spec, particles);

    // Print each cell.
    for (int y = 0; y < canvas_length; y++)
    {
        for (int x = 0; x < canvas_length; x++)
        {
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
void print_particle(Particle particle)
{
    LL_VERBOSE(
        "  Size: %d; Mass: %0.2Lf; Radius: %0.2Lf; Position: (%0.2Lf, %0.2Lf)",
        particle.size,
        particle.mass,
        particle.radius,
        particle.x,
        particle.y);
}

/**
 * Debug prints all particles.
 */
void print_particles(long long n, Particle *particles)
{
    if (log_level < LOG_LEVEL_DEBUG)
        return;

    LL_DEBUG("%s: ", "Generated particles");

    for (long long i = 0; i < n; i++)
        LL_DEBUG(
            "  Size: %d; Mass: %Lf; Radius: %Lf; Position: (%Lf, %Lf)",
            particles[i].size,
            particles[i].mass,
            particles[i].radius,
            particles[i].x,
            particles[i].y);
}
