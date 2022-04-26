#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "bmp.h"

// Every C file needs some idiosyntratic #defines
#define PAD_TO(size, align) ((((((size)-1) / align)+1) * align))
#define PTR_BYTE_ADD(p, x) ((void*)(((char*)(p))+(x)))
#define MAX(a, b) ((a) >= (b) ? (a) : (b))
#define MIN(a, b) ((a) <= (b) ? (a) : (b))
#define SWAP(a, b) { typeof(a) tmp = a; a = b; b = tmp; }
#define ROWWIDTH(width) (PAD_TO((width)*sizeof(BitmapPixel), 4))
#define COLORCLAMP(i) (uint8_t)((i) < 0 ? 0 : ((i) > 255 ? 255 : (i)))

////////////////////////////////////////////////////////////////////////////////
// Helpers /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Returns the y value associated with x on the line from (x1, y1) to (x2, y2).
static int internal_pt_on_line(
        int x1, int y1,
        int x2, int y2,
        int x) {
    // * If x1=x2, returns y1.
    // * Will suffer from numerical issues from massive ints.
    
    // Check bounds
    if (x1 == x2) {
        return y1;
    }

    // Ensure x1 > x2.
    if (x1 > x2) {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }

    int ret = x - x1;
    ret *= y2 - y1;
    ret /= x2 - x1;
    ret += y1;

    return ret;
}

static inline void internal_getrgbpixel(BitmapImage* B,
        unsigned int x, unsigned int y, BitmapPixel* px) {
    assert(x >= 0);
    assert(y >= 0);
    assert(x < B->width);
    assert(y < B->height);

    unsigned int pixel_offset = ROWWIDTH(B->width) * y;
    pixel_offset += x * sizeof(BitmapPixel);
    BitmapPixel* P = PTR_BYTE_ADD(B->raw, pixel_offset);
    px->r = P->r;
    px->g = P->g;
    px->b = P->b;
}

static inline void internal_drawrgbpixel(BitmapImage* B,
        unsigned int x, unsigned int y,
        uint8_t r, uint8_t g, uint8_t b) {
    assert(x >= 0);
    assert(y >= 0);
    assert(x < B->width);
    assert(y < B->height);

    unsigned int pixel_offset = ROWWIDTH(B->width) * y;
    pixel_offset += x * sizeof(BitmapPixel);
    BitmapPixel* P = PTR_BYTE_ADD(B->raw, pixel_offset);
    P->r = r;
    P->g = g;
    P->b = b;
}

////////////////////////////////////////////////////////////////////////////////
// Functional specification of drawing functions ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// A drawing function takes:
//
// (BitmapImage*, DrawFn*, unsigned int x, unsigned int y)

typedef void (*DrawFn_px)(BitmapImage*, DrawFn*, unsigned int, unsigned int);
typedef void (*DrawFn_del)(DrawFn*);

static inline void internal_drawpx(BitmapImage* B, DrawFn* d,
        unsigned int x, unsigned int y) {
    (*(DrawFn_px)(d->pxfn))(B, d, x, y);
}

void DrawFn_free(DrawFn* d) {
    if (d->freefn) {
        (*(DrawFn_del)(d->freefn))(d);
    }
    free(d);
}

void DrawFn_drawpx_noop(BitmapImage* B, DrawFn* d,
        unsigned int x, unsigned int y) {
    // This space intentionally left empty
}

void DrawFn_drawpx_invert(BitmapImage* B, DrawFn* d,
        unsigned int x, unsigned int y) {
    BitmapPixel px;
    internal_getrgbpixel(B, x, y, &px);
    internal_drawrgbpixel(B, x, y, 255-px.r, 255-px.g, 255-px.b);
}

DrawFn* DrawFn_init_invert() {
    DrawFn* d = malloc(sizeof(DrawFn));
    assert(d);
    d->pxfn = &DrawFn_drawpx_invert;
    d->freefn = NULL;
    return d;
}

DrawFn* DrawFn_init_none() {
    // TODO : as an optimization, have each instance of this function return the
    // same object.
    DrawFn* d = malloc(sizeof(DrawFn));
    assert(d);
    d->pxfn = &DrawFn_drawpx_noop;
    d->freefn = NULL;
    return d;
}

void DrawFn_drawpx_rgb(BitmapImage* B, DrawFn* d,
        unsigned int x, unsigned int y) {
    internal_drawrgbpixel(B, x, y, d->r1, d->g1, d->b1);
}

DrawFn* DrawFn_init_rgb(uint8_t r, uint8_t g, uint8_t b) {
    DrawFn* d = malloc(sizeof(DrawFn));
    assert(d);
    d->pxfn = &DrawFn_drawpx_rgb;
    d->freefn = NULL;
    d->r1 = r;
    d->g1 = g;
    d->b1 = b;
    return d;
}

uint8_t color_on_gradient(int i, int imax, uint8_t color1, uint8_t color2) {
    if (imax == 0) { // no data to determine intensity
        return color1;
    }
    int colordiff = (int)color2 - (int)color1;
    colordiff = colordiff * i / imax;
    return COLORCLAMP(color1 + colordiff);
}

void DrawFn_drawpx_axialgradient(BitmapImage* B, DrawFn* d,
        unsigned int x, unsigned int y) {
    int i, imax;
    // Calculate the distance in x (or y, on horizontal lines) from the main
    // line to the line through the current pixel and the other line.
    if (d->y1 != d->y2) {
        // Calculate differences in x between the control line and the other
        // two.  Do this by calculating the control line's x at y3 and
        // subtracting.
        imax = internal_pt_on_line(d->y1, d->x1, d->y2, d->x2, d->y3);
        imax = d->x3 - imax;
        i = internal_pt_on_line(d->y1, d->x1, d->y2, d->x2, y);
        i = x - i;
    } else { // horizontal
        imax = d->y3 - d->y1;
        i = y - d->y1;
    }

    // Now use the intensities to determine colors
    uint8_t r = color_on_gradient(i, imax, d->r1, d->r2);
    uint8_t g = color_on_gradient(i, imax, d->g1, d->g2);
    uint8_t b = color_on_gradient(i, imax, d->b1, d->b2);
    internal_drawrgbpixel(B, x, y, r, g, b);
}

DrawFn* DrawFn_init_axialgradient(int x1, int y1, int x2, int y2, int x3, int y3,
        uint8_t r1, uint8_t g1, uint8_t b1,
        uint8_t r2, uint8_t g2, uint8_t b2) {
    // Initialize memory
    DrawFn* d = malloc(sizeof(DrawFn));
    assert(d);

    // Initialize members
    d->pxfn = &DrawFn_drawpx_axialgradient;
    d->freefn = NULL;
    d->x1 = x1;
    d->x2 = x2;
    d->x3 = x3;
    d->y1 = y1;
    d->y2 = y2;
    d->y3 = y3;
    d->r1 = r1;
    d->g1 = g1;
    d->b1 = b1;
    d->r2 = r2;
    d->g2 = g2;
    d->b2 = b2;

    return d;
}

////////////////////////////////////////////////////////////////////////////////
// Bitmap drawing //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void bmp_drawrect(BitmapImage* B, 
        unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        DrawFn* d) {
    // Check bounds
    assert(x >= 0);
    assert(y >= 0);
    assert(x+w <= B->width);
    assert(y+h <= B->height);

    // Scan across image
    for (unsigned int j = 0; j < h; j++) {
        for (unsigned int i = 0; i < w; i++) {
            internal_drawpx(B, d, x+i, y+j);
        }
    }
}

void bmp_drawtriangle(BitmapImage* B,
        unsigned int x1, unsigned int y1,
        unsigned int x2, unsigned int y2,
        unsigned int x3, unsigned int y3,
        DrawFn* d) {
    // Borrowing the below algorithm:
    // https://www.gabrielgambetta.com/computer-graphics-from-scratch/07-filled-triangles.html

    // Check bounds
    //
    // This could be optimized, but it's nice to see exactly what's wrong.
    assert(x3 >= 0);
    assert(y1 >= 0);
    assert(y2 >= 0);
    assert(y3 >= 0);
    assert(x1 < B->width);
    assert(x2 < B->width);
    assert(x3 < B->width);
    assert(y1 < B->height);
    assert(y2 < B->height);
    assert(y3 < B->height);

    // Sort points such that y1 <= y2 <= y3
    if (y1 > y2) {
        SWAP(y1, y2);
        SWAP(x1, x2);
    } if (y1 > y3) {
        SWAP(y1, y3);
        SWAP(x1, x3);
    } if (y2 > y3) {
        SWAP(y2, y3);
        SWAP(x2, x3);
    }

    // For each horizontal line in the triangle, fill from left to right.
    for (unsigned int y = y1; y <= y3; y++) {
        // xl is the "long" side (1 to 3), xs is the other side.
        unsigned int xl = internal_pt_on_line(y1, x1, y3, x3, y);
        unsigned int xs = 0;
        if (y > y2) { // on (2, 3)
            xs = internal_pt_on_line(y2, x2, y3, x3, y);
        } else { // on (1, 2)
            xs = internal_pt_on_line(y1, x1, y2, x2, y);
        }
        int xright = MAX(xl, xs);
        int xleft  = MIN(xl, xs);
        for (int x = xleft; x <= xright; x++) {
            internal_drawpx(B, d, x, y);
        }
    }
}
