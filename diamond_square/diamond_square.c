// 
// An implementation of the square-diamond algorithm for surface (eg terrain)
// generation (also referred to as the cloud fractal)  Generates a heightmap on
// an nxn array (where n = 2^m + 1 for some natural n).  After setting the four
// corners of the array to initial values, then alternates between diamond and
// square steps until all points are initialized.  In the diamond step, set the
// center of each square to be the average of its corners plus a random value.
// In the square step, set the center of each diamond to be the average of its
// corners plus a random value.
//
// (https://en.wikipedia.org/wiki/Diamond-square_algorithm)
//

#ifndef _DIAMONDSQUARE_H_
#define _DIAMONDSQUARE_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "bmp.h"

typedef struct diamond_square {
    int** topography;
    int maxh;       // Maximum value of a height
    int dim;        // Array row/col count
    int max_steps;  // The total number of steps that can be taken
} ds_t;

#define round(x) (int)(x+0.5)
#define random() ((double)rand()) / (double)INT_MAX // Random value on [0,1]

////////////////////////////////////////////////////////////////////////////////
// The diamond-square algorithm ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// N steps fill (2^n)+1 arrays
// For example, a 0-step is 2x2 (a square), 1-step 3x3, 2-step 5x5, etc

int dim_from_steps(int steps) {
    return (1 << steps) + 1;
}

int steps_from_dim(int dim) {
    int steps = 0;
    dim -= 1;
    dim >>= 1;
    while (dim) {
        dim >>= 1;
        steps++;
    }
    return steps;
}

void d_s_recurse(ds_t* T,int steps_remaining,int random_magnitude) {
    if (steps_remaining <= 0) return;
    int step_size = (1 << steps_remaining) >> 1; // Step size is 2^(n-1), or
    // Diamond step                              // half the side len of square
    // Set the center of the diamond equal to the average of the four 
    // corners plus a random offset
    for (int i = step_size; i < T->dim; i += 2*step_size) {
        for (int j = step_size; j < T->dim; j += 2*step_size) {
            int r = round(random() * random_magnitude) - (random_magnitude / 2);
            int sum_elev = T->topography[i - step_size][j - step_size] +
                T->topography[i + step_size][j - step_size] +
                T->topography[i - step_size][j + step_size] +
                T->topography[i + step_size][j + step_size];
            int height = (sum_elev / 4) + r;
            if (height < 0) height = 0;
            if (height > T->maxh) height = T->maxh;
            T->topography[i][j] = height;
        }
    }

    
    // Square step
    for (int i = 0; i < T->dim; i += step_size) {
        int j = (!((i / step_size) % 2)) * step_size;
        for (; j < T->dim; j += 2*step_size) {
            int r = round(random() * random_magnitude) - (random_magnitude / 2);
            int n_corners = (i != 0) + (j != 0) + (i != T->dim - 1) + (j != T->dim - 1);

            int sum_elev = (i != 0) ? T->topography[i - step_size][j] : 0;
            sum_elev += (j != 0) ? T->topography[i][j - step_size] : 0;
            sum_elev += (i != T->dim - 1) ? T->topography[i + step_size][j] : 0;
            sum_elev += (j != T->dim - 1) ? T->topography[i][j + step_size] : 0;

            int height = (sum_elev / 4) + r;
            if (height < 0) height = 0;
            if (height > T->maxh) height = T->maxh;
            T->topography[i][j] = height;
        }
    }
    d_s_recurse(T,steps_remaining - 1, random_magnitude / 2);

    return;
}

// Runs the Diamond-Square algorithm to generate a surface
void d_s(ds_t* T) {
    // Use constant initializations for the corners
    int initial = T->maxh / 2;
    /*T->topography[0][0] = initial;
    T->topography[0][T->dim-1] = initial;
    T->topography[T->dim-1][0] = initial;
    T->topography[T->dim-1][T->dim-1] = initial;*/
    T->topography[0][0] = (int)(random() * T->maxh);
    T->topography[0][T->dim-1] = (int)(random() * T->maxh);
    T->topography[T->dim-1][0] = (int)(random() * T->maxh);
    T->topography[T->dim-1][T->dim-1] = (int)(random() * T->maxh);

    return d_s_recurse(T,T->max_steps,T->maxh / 2);
}


ds_t* new_ds(unsigned int pxdim, int maxh, int square_size) {
    ds_t* T = malloc(sizeof(ds_t));

    int min_dim = pxdim / square_size;
    T->maxh = maxh;
    T->max_steps = steps_from_dim(min_dim);
    T->dim = dim_from_steps(T->max_steps);

    T->topography = malloc(sizeof(int*) * T->dim);
    for (int i = 0; i < T->dim; i++) T->topography[i] = malloc(sizeof(int) * T->dim);

    d_s(T);

    return T;
}

void free_ds(ds_t* T) {
    for (int i = 0; i < T->dim; i++) free(T->topography[i]);
    free(T->topography);
    free(T);
    return;
}

////////////////////////////////////////////////////////////////////////////////
// Drawing functions ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Maximum and minimum intensity values
#define INTENSITY_MIN 0.0
#define INTENSITY_MAX 1.0

/* Returns a scaled intensity value (assuming that T->maxh is the max height
 * (equal to 1.0 = white) and 0 is the min height (equal to 0.0 = black).
 */
double scaled_col(ds_t* T, int height) {
    const double intensity_range = INTENSITY_MAX - INTENSITY_MIN;
    return (intensity_range * (double)height / (double)(T->maxh)) + INTENSITY_MIN;
}

uint8_t blend(uint8_t c1, uint8_t c2, double intensity) {
    // Assumes that intensity is on (0, 1)
    assert(intensity >= 0.0);
    assert(intensity <= 1.0);
    int colordiff = (int)c2 - (int)c1;
    colordiff = (int)((double)colordiff * intensity);
    colordiff = colordiff + c1;
    assert(colordiff >= 0 && colordiff < 256);
    return (uint8_t)colordiff;
}

void DrawFn_drawpx_diamondsquare(BitmapImage* B, DrawFn* d,
        unsigned int x, unsigned int y) {
    // Check bounds
    if (x >= d->x1 + d->x2 || y >= d->y1 + d->x2) return;

    // Init
    ds_t* T = d->mem;

    // Calculate which square this pixel is in (identified by top-left corner)
    int square_x = (x - d->x1) * (T->dim-1) / d->x2;
    int square_y = (y - d->y1) * (T->dim-1) / d->x2;
    assert(square_x < (T->dim - 1));
    assert(square_y < (T->dim - 1));

    // Calculate the color of this pixel from the bounding points.
    //
    // Calculate distance from each corner, and use these for a weighted average
    // of intensities.  Use Manhattan distance for simplicity.
    //
    // TODO : this approach actually doesn't work, so we just use a pixel
    // stride of 1 in building the DS structure.  Get this approach working in
    // the future.
    double intensity = scaled_col(T, T->topography[square_x][square_y]);

    uint8_t rdiff = d->r2 - d->r1;
    uint8_t r = blend(d->r1, d->r2, intensity);
    uint8_t g = blend(d->g1, d->g2, intensity);
    uint8_t b = blend(d->b1, d->b2, intensity);

    // Draw the pixel
    bmp_drawrgbpixel(B, x, y, r, g, b);
}

void DrawFn_free_diamondsquare(DrawFn* d) {
    free_ds(d->mem);
}

DrawFn* DrawFn_init_diamondsquare(int x, int y, int w, int h,
        uint8_t r1, uint8_t g1, uint8_t b1,
        uint8_t r2, uint8_t g2, uint8_t b2) {
    // Initialize memory
    DrawFn* d = malloc(sizeof(DrawFn));
    assert(d);

    // Initialize functions
    d->pxfn = &DrawFn_drawpx_diamondsquare;
    d->freefn = &DrawFn_free_diamondsquare;

    // Initialize other members and run the Diamond-Square algorithm.
    //
    // Use x1, y1 to store top left; x2 to store w & h.
    d->x1 = x;
    d->y1 = y;
    d->x2 = (w > h) ? w : h;
    d->mem = new_ds(d->x2, 1<<12, 1);
    d->r1 = r1;
    d->g1 = g1;
    d->b1 = b1;
    d->r2 = r2;
    d->g2 = g2;
    d->b2 = b2;
    return d;
}

//////////////////////////////////////////////////////////////////////////////////
//// OLD CODE TO BE DELETED///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//
//
//#include <stdbool.h>
//
///* Draws a gradient square with intensity i0 at (x0,y0), i1 at (x0,y1), i2 at (x1,y1), i3 at (x1,y0)
// * to cr.  Returns true on succes, false on failure
// */
//bool draw_square(cairo_t* cr, int x0, int y0, int x1, int y1, double i0, double i1, double i2, double i3) {
//    // Create the pattern
//    cairo_pattern_t* pat = cairo_pattern_create_mesh();
//    cairo_mesh_pattern_begin_patch(pat);
//    
//    cairo_mesh_pattern_move_to(pat,x0,y0);
//    cairo_mesh_pattern_set_corner_color_rgb(pat,0,i0,i0,i0);
//
//    cairo_mesh_pattern_line_to(pat,x0,y1);
//    cairo_mesh_pattern_set_corner_color_rgb(pat,1,i1,i1,i1);
//    
//    cairo_mesh_pattern_line_to(pat,x1,y1);
//    cairo_mesh_pattern_set_corner_color_rgb(pat,2,i2,i2,i2);
//    
//    cairo_mesh_pattern_line_to(pat,x1,y0);
//    cairo_mesh_pattern_set_corner_color_rgb(pat,3,i3,i3,i3);
//    
//    cairo_mesh_pattern_end_patch(pat);
//
//    if (cairo_pattern_status(pat) != CAIRO_STATUS_SUCCESS) return false;
//
//    cairo_rectangle(cr, x0, y0, x1, y1);
//    cairo_set_source(cr, pat);
//    cairo_fill(cr);
//    
//    // Clean up
//    cairo_pattern_destroy(pat);
//    return true;
//}
//
//// TODO : errorcheck properly
///* Draws surface out to file filename, a png of width width and height height
// *
// * Returns true on success, false on failure.
// */
//bool draw_ds(ds_t* T,const char* filename,int width,int height) {
//    // Initialize
//    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height);
//    cairo_t* cr = cairo_create(surface);
//
//    // Loop over the array, creating squares
//    for (int i = 0; i < T->dim-1; i++) {
//        int x0 = (width * i) / (T->dim-1);
//        int x1 = (width * (i+1)) / (T->dim-1);
//        for (int j = 0; j < T->dim-1; j++) {
//            int y0 = height * j / (T->dim-1);
//            int y1 = height * (j+1) / (T->dim-1);
//            double i0 = scaled_col(T,T->topography[i][j]);
//            double i1 = scaled_col(T,T->topography[i][j+1]);
//            double i2 = scaled_col(T,T->topography[i+1][j+1]);
//            double i3 = scaled_col(T,T->topography[i+1][j]);
//            if (!draw_square(cr,x0,y0,x1,y1,i0,i1,i2,i3)) return false;
//        }
//    }
//
//    // Write to png
//    cairo_surface_write_to_png(surface,filename);
//
//    // Clean up
//    cairo_destroy(cr);
//    cairo_surface_destroy(surface);
//
//    return true;
//}

#endif /* _DIAMONDSQUARE_H_ */
