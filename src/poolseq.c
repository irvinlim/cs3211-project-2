/**
 * poolseq.c
 * 
 * For the sequential version of the program, we will run the 
 * entire procedure on a single process, although the number of
 * cores (supplied by the -np flag) will be used to determine the
 * number of regions that should be computed.
 */

#include <assert.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simulation/nbody.h"
#include "utils/common.h"
#include "utils/env.h"
#include "utils/heatmap.h"
#include "utils/log.h"
#include "utils/multiproc.h"
#include "utils/particles.h"
#include "utils/regions.h"
#include "utils/spec.h"
#include "utils/timer.h"

#define PROG "poolseq"
#define TIMEBUF_LENGTH 10

// Stores the specifications for the program.
Spec spec;

// Store the total computation and communication time for all iterations.
long long comp_sum = 0;

/**
 * Generates canvases for each region so that we can generate a PPM heatmap.
 */
void collate_generate_heatmap(int *sizes, Particle **particles_by_region, char *outputfile)
{
    int num_cores = get_num_cores();

    // Bail early if we have no output filename.
    if (outputfile == NULL) return;

    // Allocate space for all canvases.
    int ***all_canvases = malloc(num_cores * sizeof(int **));

    // Generate canvas for each region.
    for (int region_id = 0; region_id < num_cores; region_id++) {
        print_particles(LOG_LEVEL_DEBUG, "Generating canvas for my region", sizes[region_id], particles_by_region[region_id]);
        all_canvases[region_id] = generate_region_canvas(spec, sizes[region_id], particles_by_region[region_id], region_id);
        LL_VERBOSE("Canvas generated for region %d.", region_id);
    }

    // Generate the heatmap.
    generate_heatmap(spec, num_cores, all_canvases, outputfile);
    free(all_canvases);
}

/**
 * Generates a debug frame and saves it to the frames directory.
 */
void generate_debug_frame(int frame_id, int *sizes, Particle **particles, char *framesdir)
{
    int outputdir_len = strlen(framesdir);
    char *outputfile = malloc(outputdir_len + 20);
    sprintf(outputfile, "%s/%d.ppm", framesdir, frame_id);

    FILE *fp = fopen(outputfile, "w");
    if (fp == NULL) {
        LL_ERROR("Could not open %s for writing debug frame to!", outputfile);
        exit(EXIT_FAILURE);
    }

    collate_generate_heatmap(sizes, particles, outputfile);
}

/**
 * Runs a single time step for all regions.
 */
Particle **execute_time_step(int *sizes, Particle **particles_by_region)
{
    int num_cores = get_num_cores();
    long double dt = spec.TimeStep;

    // Compute new velocities for all regions. Horizon is ignored for sequential computation.
    for (int i = 0; i < num_cores; i++)
        update_velocity(dt, spec, sizes, particles_by_region, num_cores, i);

    // Handle collisions for all particles, updating the velocity (direction) if necessary.
    for (int i = 0; i < num_cores; i++)
        handle_collisions(spec, sizes, particles_by_region, num_cores, i);

    // Handle collisions of particles against the walls of the pool.
    for (int i = 0; i < num_cores; i++)
        handle_wall_collisions(spec, sizes[i], particles_by_region[i], i);

    // Update the positions of all particles in all regions.
    for (int i = 0; i < num_cores; i++)
        update_position(dt, spec, sizes[i], particles_by_region[i], i);

    // Reallocate the particles into their correct regions.
    Particle ***reallocated_particles = malloc(num_cores * sizeof(Particle **));
    int **reallocated_sizes = malloc(num_cores * sizeof(int *));
    for (int i = 0; i < num_cores; i++)
        reallocated_sizes[i] = calloc(num_cores, sizeof(int));
    for (int i = 0; i < num_cores; i++)
        reallocated_particles[i] = reallocate_for_region(spec, reallocated_sizes[i], sizes[i], particles_by_region[i], num_cores);

    for (int i = 0; i < num_cores; i++)
        for (int j = 0; j < num_cores; j++)
            print_particle_ids(LOG_LEVEL_DEBUG, "Dump of reallocated particles", reallocated_sizes[i][j], reallocated_particles[i][j]);

    // Recompute the sizes for all regions based on what was allocated.
    for (int i = 0; i < num_cores; i++)
        sizes[i] = 0;
    for (int i = 0; i < num_cores; i++)
        for (int j = 0; j < num_cores; j++)
            sizes[j] += reallocated_sizes[i][j];

    print_ints(LOG_LEVEL_DEBUG, "Resultant region sizes", num_cores, sizes);

    // Merge all regions back together.
    Particle **merged_particles = allocate_particles(sizes, num_cores);
    int *counters = calloc(num_cores, sizeof(int));
    for (int process = 0; process < num_cores; process++) // "Process" number
        for (int region = 0; region < num_cores; region++) // Region within the "process"
            for (int k = 0, size = reallocated_sizes[process][region]; k < size; k++) // Particle within the region
                merged_particles[region][counters[region]++] = reallocated_particles[process][region][k];

    print_ints(LOG_LEVEL_DEBUG, "Merged region sizes", num_cores, counters);

    // Free all dynamically allocated memory.
    for (int i = 0; i < num_cores; i++) free(reallocated_sizes[i]);
    free(reallocated_sizes);
    free(counters);
    deallocate_particles(particles_by_region, num_cores);

    return merged_particles;
}

/**
 * Runs the simulation according to the provided specifications.
 */
Particle **run_simulation(int *sizes, Particle **particles_by_region, char *framesdir)
{
    long long start, end;
    char timebuf[TIMEBUF_LENGTH];

    for (int i = 0; i < spec.TimeSlots; i++) {
        // If debugging of frames is enabled, generate a frame and save it to the frames directory.
        if (framesdir != NULL) generate_debug_frame(i, sizes, particles_by_region, framesdir);

        // Execute time step.
        start = wall_clock_time();
        particles_by_region = execute_time_step(sizes, particles_by_region);
        end = wall_clock_time();
        comp_sum += end - start;
        format_time(timebuf, TIMEBUF_LENGTH, end - start);
        LL_VERBOSE("Computation time for iteration %4.0d: %s seconds", i + 1, timebuf);
    }

    return particles_by_region;
}

/**
 * Collates timings for all processes, calculates the average
 * and generates a report.
 */
void collate_timings(char *reportfile)
{
    int num_cores = get_num_cores();
    char timebuf[TIMEBUF_LENGTH];

    // Print the report on the master process.
    if (is_master()) {
        LL_SUCCESS("%s", "============================");
        LL_SUCCESS("%s", "    Pool Simulator Report   ");
        LL_SUCCESS("%s", "============================");
        LL_SUCCESS("Number of regions:    %d", num_cores);
        LL_SUCCESS("Number of iterations: %d", spec.TimeSlots);
        LL_SUCCESS("Particles per region: %d", spec.TotalNumberOfParticles);
        LL_SUCCESS("%s", "============================");
        LL_SUCCESS("%s", "Computation time:");
        format_time(timebuf, TIMEBUF_LENGTH, comp_sum);
        LL_SUCCESS("+ Sum: %s seconds", timebuf);
        format_time(timebuf, TIMEBUF_LENGTH, comp_sum / spec.TimeSlots);
        LL_SUCCESS("+ Avg: %s seconds", timebuf);
        LL_SUCCESS("%s", "============================");

        // Write the report to a file if filename was specified.
        if (reportfile != NULL) {
            FILE *fp = fopen(reportfile, "w");
            if (fp == NULL) {
                LL_ERROR("Could not open report file %s for writing!", reportfile);
                return;
            }

            fprintf(fp, "%s\n", "============================");
            fprintf(fp, "%s\n", "    Pool Simulator Report   ");
            fprintf(fp, "%s\n", "============================");
            fprintf(fp, "Number of regions:    %d\n", num_cores);
            fprintf(fp, "Number of iterations: %d\n", spec.TimeSlots);
            fprintf(fp, "Particles per region: %d\n", spec.TotalNumberOfParticles);
            fprintf(fp, "%s\n", "============================");
            fprintf(fp, "%s\n", "Computation time:");
            format_time(timebuf, TIMEBUF_LENGTH, comp_sum);
            fprintf(fp, "+ Sum: %s seconds\n", timebuf);
            format_time(timebuf, TIMEBUF_LENGTH, comp_sum / spec.TimeSlots);
            fprintf(fp, "+ Avg: %s seconds\n", timebuf);
            fprintf(fp, "%s\n", "============================");

            LL_SUCCESS("Pool simulator report was saved to %s.", reportfile);

            fclose(fp);
        }
    }
}

/**
 * Main method for the structue of the entire program.
 */
void start(char *specfile, char *outputfile, char *reportfile, char *framesdir)
{
    int num_cores = get_num_cores();

    LL_NOTICE("Starting %s with %d region(s) on %d processor(s)...", PROG, get_num_cores(), get_num_cores());

    // Allocate space for particles and their array sizes.
    int *sizes = calloc(num_cores, sizeof(int));
    Particle **particles_by_region = allocate_particles(sizes, num_cores);

    for (int i = 0; i < num_cores; i++) {
        // Read the specification file, which will generate large particles.
        spec = read_spec_file(i, specfile);

        // Generate particles for this region.
        particles_by_region[i] = generate_particles(i, spec);
        sizes[i] = spec.TotalNumberOfParticles;
    }

    // Debug print all read-in values.
    print_spec(spec);
    print_canvas_info(spec);

    // Run the simulation only in the region assigned.
    LL_NOTICE("Simulation is starting on %d core(s).", get_num_cores());
    particles_by_region = run_simulation(sizes, particles_by_region, framesdir);
    LL_NOTICE("%s", "Simulation completed!");

    // Collate timings from all processes to generate a report.
    collate_timings(reportfile);

    // Collate particles and generate the heatmap on the master process.
    collate_generate_heatmap(sizes, particles_by_region, outputfile);
}

int main(int argc, char **argv)
{
    multiproc_init(argc, argv);
    set_log_level_env();

    // Parse arguments.
    check_arguments(argc, PROG);
    char *specfile = argv[1];
    char *outputfile = argv[2];
    char *reportfile = NULL;
    if (argc == 4) reportfile = argv[3];
    char *framesdir = NULL;
    if (argc == 5) framesdir = argv[4];

    // Only run on a single (master) process; other processes can terminate.
    if (is_master()) start(specfile, outputfile, reportfile, framesdir);

    multiproc_finalize();
    return 0;
}
