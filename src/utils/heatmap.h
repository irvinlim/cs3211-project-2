#include "types.h"

/**
 * Generates a 2-D canvas of all particles.
 * 
 * Any values less than or equal to BITMAP_MAX represents
 * the presence of the body of a small particle, while
 * any value greater than BITMAP_MAX represents the body
 * of a large particle.
 */
int **generate_canvas(Spec spec, Particle *particles);

/**
 * Generate a heatmap of particles and saves it to an image file.
 */
void generate_heatmap(Spec spec, Particle *particles, char *outputfile);
