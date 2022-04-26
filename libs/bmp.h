#ifndef _BMP_H_
#define _BMP_H_

#include <stdint.h>
#include "bmp_base.h"

////////////////////////////////////////////////////////////////////////////////
// Drawing functions ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef struct {
    void* pxfn;
    void* freefn;
    uint8_t r1, g1, b1, r2, g2, b2;
    int x1, y1, x2, y2, x3, y3;
    void* mem;
} DrawFn;

// Cleans up memory used by a DrawFn;
void DrawFn_free(DrawFn* d);

// Draws a solid color
DrawFn* DrawFn_init_rgb(uint8_t r, uint8_t g, uint8_t b);

// Inverts colors it draws over
DrawFn* DrawFn_init_invert();

// Does nothing
DrawFn* DrawFn_init_none();

// Draws a gradient between two axes (x1, y1) to (x2, y2) and a line with
// the same slope through (x3, y3).  Behavior may be strange if not between the
// two lines.
DrawFn* DrawFn_init_axialgradient(
        int x1, int y1, int x2, int y2, int x3, int y3,
        uint8_t r1, uint8_t g1, uint8_t b1,
        uint8_t r2, uint8_t g2, uint8_t b2);

////////////////////////////////////////////////////////////////////////////////
// Write to a bitmap ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// All of these functions do some error checking via assert.

// Draw a rectangle (specified by top left, width, and height)
void bmp_drawrect(BitmapImage* B, 
        unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        DrawFn* d);

// Draw a triangle (specified by vertices)
void bmp_drawtriangle(BitmapImage* B,
        unsigned int x1, unsigned int y1,
        unsigned int x2, unsigned int y2,
        unsigned int x3, unsigned int y3,
        DrawFn* d);

#endif /* _BMP_H_ */
