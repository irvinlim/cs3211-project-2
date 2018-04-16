#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "particles.h"
#include "regions.h"

#define BITMAP_MAX 255

/**
 * Generates a 2-D canvas of all particles.
 * 
 * Any values less than or equal to BITMAP_MAX represents
 * the presence of the body of a small particle, while
 * any value greater than BITMAP_MAX represents the body
 * of a large particle.
 */
int **generate_region_canvas(Spec spec, int n, Particle *particles, int region_id)
{
    int canvas_length = spec.GridSize * spec.PoolLength;

    // Allocate enough memory for a 2-D canvas.
    int **canvas = (int **)malloc(canvas_length * sizeof(int *));
    for (int i = 0; i < canvas_length; i++)
        canvas[i] = (int *)calloc(canvas_length, sizeof(int));

    // Iterate through all particles.
    for (int i = 0; i < n; i++) {
        Particle p = particles[i];

        // Round up the coordinates of the particle's radius to find bounding box.
        int r = ceil(p.radius);

        // Iterate through all pixels occupied by the particle using simple
        // radius checking in the bounding box.
        for (int j = -r; j <= r; j++) {
            for (int k = -r; k <= r; k++) {
                if (j * j + k * k < p.radius * p.radius) {
                    // Denormalize the position wrt region.
                    int nx = denorm_region_x(p.x, region_id, spec);
                    int ny = denorm_region_y(p.y, region_id, spec);

                    // Get the coordinates relative to the origin.
                    int x = nx + k;
                    int y = ny + j;

                    // Prevent drawing outside of the bounds of the board.
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
 * Generate a heatmap of particles in all regions from a list of canvases, 
 * and saves it to an image file.
 */
void generate_heatmap(Spec spec, int num_regions, int ***canvas_by_region, char *outputfile)
{
    int canvas_length = spec.GridSize * spec.PoolLength;

    // Open file for writing.
    FILE *fp = fopen(outputfile, "w");
    if (fp == NULL) {
        LL_ERROR("Could not open %s for writing!", outputfile);
        exit(EXIT_FAILURE);
    }

    // Print PPM header.
    fprintf(fp, "P3\n%d %d\n%d\n", canvas_length, canvas_length, BITMAP_MAX);

    // Print each cell in the entire canvas.
    for (int y = 0; y < canvas_length; y++) {
        for (int x = 0; x < canvas_length; x++) {
            int sum = 0;

            // Sum up the cells.
            for (int region = 0; region < num_regions; region++) {
                int cell = canvas_by_region[region][y][x];

                if (sum > BITMAP_MAX || cell > BITMAP_MAX)
                    sum = BITMAP_MAX + 1;
                else
                    sum = fmin(sum + cell, BITMAP_MAX);
            }

            // If the value is greater than BITMAP_MAX, we draw a blue pixel.
            // Otherwise, we draw a red pixel whose intensity is the value.
            if (sum > BITMAP_MAX)
                fprintf(fp, "0 0 %d", BITMAP_MAX);
            else
                fprintf(fp, "%d 0 0", sum);

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
