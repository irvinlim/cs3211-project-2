#include "types.h"

/**
 * Generates a 2-D canvas of all particles in a particular region.
 * 
 * Any values less than or equal to BITMAP_MAX represents
 * the presence of the body of a small particle, while
 * any value greater than BITMAP_MAX represents the body
 * of a large particle.
 */
int **generate_region_canvas(Spec spec, int n, Particle *particles, int region_id);

/**
 * Generate a heatmap of particles in all regions from a list of canvases, 
 * and saves it to an image file.
 */
void generate_heatmap(Spec spec, int num_regions, int ***canvas_by_region, char *outputfile);
