#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "particles.h"

#if defined(DEBUG_PARTICLES) && DEBUG_PARTICLES > 0
#define DEBUG_PRINT(x) printf x
#else // clang-format off
#define DEBUG_PRINT(x) do {} while (0);
#endif // clang-format on

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
 * Count the number of small particles in each cell in all regions.
 */
int **count_small_particles(Spec spec, Particle *particles)
{
    long long i;
    long x, y;
    int **canvas;

    // Get canvas length.
    long canvas_length = spec.GridSize * spec.PoolLength;

    // Allocate enough memory for a 2-D canvas.
    canvas = (int **)malloc(canvas_length * sizeof(int *));
    for (i = 0; i < canvas_length; i++)
        canvas[i] = (int *)calloc(canvas_length, sizeof(int));

    // Iterate through all particles.
    for (i = 0; i < spec.TotalNumberOfParticles; i++)
    {
        // Only count small particles.
        if (particles[i].size != SMALL)
            continue;

        // Round down the coordinates of the particle.
        x = (int)floor(particles[i].x);
        y = (int)floor(particles[i].y);

        // Increment the counter for the position.
        canvas[y][x]++;
    }

    return canvas;
}

/**
 * Generate a heatmap of particles and saves it to an image file.
 */
void generate_heatmap(Spec spec, Particle *particles, char *outputfile)
{
    int count;
    long i, j;
    int **canvas;

    // Get canvas length.
    long canvas_length = spec.GridSize * spec.PoolLength;

    // Open file for writing.
    FILE *fp = fopen(outputfile, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "\033[0;31mERROR:\033[0m Could not open %s for writing!\n", outputfile);
        exit(EXIT_FAILURE);
    }

    // Print header.
    fprintf(fp, "P3\n%ld %ld\n%d\n", spec.GridSize, spec.GridSize, BITMAP_MAX);

    // Count large and small particles.
    canvas = count_small_particles(spec, particles);

    // Print each row.
    for (i = 0; i < canvas_length; i++)
    {
        for (j = 0; j < canvas_length; j++)
        {
            // Cap the number at BITMAP_MAX.
            count = canvas[i][j] > BITMAP_MAX ? BITMAP_MAX : canvas[i][j];

            // Write count to the file.
            fprintf(fp, "%d 0 0", count);

            if (j < canvas_length - 1)
                fprintf(fp, " ");
            else
                fprintf(fp, "\n");
        }
    }

    // Print success message.
    printf("\033[0;32mSuccessfully written image to %s.\033[0m\n", outputfile);

    // Clean up.
    fclose(fp);
}

/**
 * Debug prints details about a particle.
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

/**
 * Debug prints all particles.
 */
void print_particles(long long n, Particle *particles)
{
    long long i;
    DEBUG_PRINT(("\033[0;36m"));

    DEBUG_PRINT(("Generated particles:\n"));
    for (i = 0; i < n; i++)
        print_particle(particles[i]);

    DEBUG_PRINT(("\033[0m\n"));
}
