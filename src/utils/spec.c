#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "multiproc.h"
#include "particles.h"
#include "spec.h"
#include "types.h"

/**
 * Reads the specification file.
 */
Spec read_spec_file(int region_id, char *specfile)
{
    // Open the specification file.
    FILE *fp = fopen(specfile, "r");
    if (fp == NULL) {
        LL_ERROR("%s not found!", specfile);
        exit(EXIT_FAILURE);
    }

    // Create SPEC struct.
    Spec spec = {
        .PoolLength = get_pool_length(),
    };

    // Read specification lines.
    // Assumes that values are in fixed format.
    fscanf(fp,
        "TimeSlots: %ld\n"
        "TimeStep: %Lf\n"
        "Horizon: %d\n"
        "GridSize: %d\n"
        "NumberOfSmallParticles: %d\n"
        "SmallParticleMass: %Lf\n"
        "SmallParticleRadius: %Lf\n"
        "NumberOfLargeParticles: %d\n",
        &spec.TimeSlots,
        &spec.TimeStep,
        &spec.Horizon,
        &spec.GridSize,
        &spec.NumberOfSmallParticles,
        &spec.SmallParticleMass,
        &spec.SmallParticleRadius,
        &spec.NumberOfLargeParticles);

    // Calculate total number of particles.
    spec.TotalNumberOfParticles = spec.NumberOfSmallParticles + spec.NumberOfLargeParticles;

    // Read large particle values, according to NumberOfLargeParticles.
    if (spec.NumberOfLargeParticles < 0) {
        LL_ERROR("%s", "NumberOfLargeParticles cannot be negative!");
        exit(EXIT_FAILURE);
    }

    spec.LargeParticles = malloc(sizeof(Particle) * spec.NumberOfLargeParticles);
    for (int i = 0; i < spec.NumberOfLargeParticles; i++) {
        fscanf(fp,
            "%Lf %Lf %Lf %Lf\n",
            &spec.LargeParticles[i].radius,
            &spec.LargeParticles[i].mass,
            &spec.LargeParticles[i].x,
            &spec.LargeParticles[i].y);
    }

    // Fill missing values for large particles.
    for (int i = 0; i < spec.NumberOfLargeParticles; i++) {
        spec.LargeParticles[i].id = region_id * spec.TotalNumberOfParticles + i;
        spec.LargeParticles[i].region = region_id;
        spec.LargeParticles[i].size = LARGE;
        spec.LargeParticles[i].vx = 0.0L;
        spec.LargeParticles[i].vy = 0.0L;
    }

    // Clean up.
    fclose(fp);

    return spec;
}

/**
 * Returns the length of the size of the pool.
 */
int get_pool_length()
{
    // Return the square root of the number of cores as the pool length.
    return (int)sqrt(get_num_cores());
}

/**
 * Debug prints the Spec.
 */
void print_spec(Spec spec)
{
    LL_VERBOSE("%s: ", "Loaded specification");
    LL_VERBOSE("- TimeSlots: %ld", spec.TimeSlots);
    LL_VERBOSE("- TimeStep: %Lf", spec.TimeStep);
    LL_VERBOSE("- Horizon: %d", spec.Horizon);
    LL_VERBOSE("- GridSize: %d", spec.GridSize);
    LL_VERBOSE("- NumberOfSmallParticles: %d", spec.NumberOfSmallParticles);
    LL_VERBOSE("- SmallParticleMass: %Lf", spec.SmallParticleMass);
    LL_VERBOSE("- SmallParticleRadius: %Lf", spec.SmallParticleRadius);
    LL_VERBOSE("- NumberOfLargeParticles: %d", spec.NumberOfLargeParticles);
    LL_VERBOSE("%s: ", "Large particle data");

    for (int i = 0; i < spec.NumberOfLargeParticles; i++)
        print_particle(LOG_LEVEL_VERBOSE, spec.LargeParticles[i]);
}

/**
 * Prints info about the canvas.
 */
void print_canvas_info(Spec spec)
{
    // Print info about canvas size.
    int total_particles = spec.TotalNumberOfParticles;
    int num_grids = spec.PoolLength * spec.PoolLength;
    int grid_size = spec.GridSize;
    int canvas_size = spec.GridSize * spec.PoolLength;
    LL_NOTICE("Generated %d particle(s) in %d region(s) of size %dx%d each; Total canvas size is %dx%d; Total number of particles is %d.",
        total_particles, num_grids, grid_size, grid_size, canvas_size, canvas_size, total_particles * num_grids);
}
