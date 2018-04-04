#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include "common.h"

#define DEBUG 1

/**
 * Prints debug statements.
 */
void debug(char *msg)
{
#if defined(DEBUG) && DEBUG > 0
#define DEBUG_PRINT(x) printf x
#else // clang-format off
#define DEBUG_PRINT(x) do {} while (0);
#endif // clang-format on
}

/** 
 * Determines the current time.
 */
long long wall_clock_time()
{
#ifdef LINUX
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (long long)(tp.tv_nsec + (long long)tp.tv_sec * 1000000000ll);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_usec * 1000 + (long long)tv.tv_sec * 1000000000ll);
#endif
}

/**
 * Checks whether the number of arguments are satisfied.
 */
void check_arguments(int argc, char **argv, char *prog)
{
    if (argc != 3)
    {
        fprintf(stderr, "\033[0;32mUsage:\033[0m mpirun %s specfile.txt outputfile.ppm\n", prog);
        exit(EXIT_FAILURE);
    }
}

/**
 * Reads the specification file.
 */
Spec read_spec_file(char *specfile)
{
    int i;

    // Open the specification file.
    FILE *fp = fopen(specfile, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "\033[0;31mERROR:\033[0m %s not found!\n", specfile);
        exit(EXIT_FAILURE);
    }

    // Create SPEC struct.
    Spec spec;

    // Read specification lines.
    // Assumes that values are in fixed format.
    fscanf(fp,
           "TimeSlots: %ld\n"
           "TimeStep: %Lf\n"
           "Horizon: %ld\n"
           "GridSize: %ld\n"
           "NumberOfSmallParticles: %lld\n"
           "SmallParticleMass: %Lf\n"
           "SmallParticleRadius: %Lf\n"
           "NumberOfLargeParticles: %lld\n",
           &spec.TimeSlots,
           &spec.TimeStep,
           &spec.Horizon,
           &spec.GridSize,
           &spec.NumberOfSmallParticles,
           &spec.SmallParticleMass,
           &spec.SmallParticleRadius,
           &spec.NumberOfLargeParticles);

    // Read large particle values, according to NumberOfLargeParticles.
    if (spec.NumberOfLargeParticles < 0)
    {
        fprintf(stderr, "\033[0;31mERROR:\033[0m NumberOfLargeParticles cannot be negative!\n");
        exit(EXIT_FAILURE);
    }

    spec.LargeParticles = malloc(sizeof(Particle) * spec.NumberOfLargeParticles);
    for (i = 0; i < spec.NumberOfLargeParticles; i++)
    {
        fscanf(fp,
               "%Lf %Lf %Lf %Lf\n",
               &spec.LargeParticles[i].mass,
               &spec.LargeParticles[i].radius,
               &spec.LargeParticles[i].x,
               &spec.LargeParticles[i].y);
    }

    // Debug print all read-in values.
    DEBUG_PRINT(("\033[0;36m"));
    DEBUG_PRINT(("Specification file read (%s):\n", specfile));
    DEBUG_PRINT(("+ TimeSlots: %ld\n", spec.TimeSlots));
    DEBUG_PRINT(("+ TimeStep: %Lf\n", spec.TimeStep));
    DEBUG_PRINT(("+ Horizon: %ld\n", spec.Horizon));
    DEBUG_PRINT(("+ GridSize: %ld\n", spec.GridSize));
    DEBUG_PRINT(("+ NumberOfSmallParticles: %lld\n", spec.NumberOfSmallParticles));
    DEBUG_PRINT(("+ SmallParticleMass: %Lf\n", spec.SmallParticleMass));
    DEBUG_PRINT(("+ SmallParticleRadius: %Lf\n", spec.SmallParticleRadius));
    DEBUG_PRINT(("+ NumberOfLargeParticles: %lld\n", spec.NumberOfLargeParticles));
    DEBUG_PRINT(("+ LargeParticles:\n"));
    for (i = 0; i < spec.NumberOfLargeParticles; i++)
    {
        DEBUG_PRINT(("+ %d: Mass: %Lf; Radius: %Lf; Position: (%Lf, %Lf)\n",
                     i + 1,
                     spec.LargeParticles[i].mass,
                     spec.LargeParticles[i].radius,
                     spec.LargeParticles[i].x,
                     spec.LargeParticles[i].y));
    }
    DEBUG_PRINT(("\033[0m"));

    // Clean up.
    fclose(fp);

    return spec;
}
