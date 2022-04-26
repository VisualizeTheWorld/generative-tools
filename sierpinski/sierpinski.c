#ifndef _SIERPINSKI_H_
#define _SIERPINSKI_H_

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
#include "sierpinski.h"

////////////////////////////////////////////////////////////////////////////////
// Sierpinski carpet ///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void recurse_sier_carpet(BitmapImage* B, unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        DrawFn* d) {
    // Draw central sub-rect.
    unsigned int x1 = x + (w/3);
    unsigned int x2 = x + (w*2/3);
    unsigned int x3 = x + w;
    unsigned int y1 = y + (h/3);
    unsigned int y2 = y + (h*2/3);
    unsigned int y3 = y + h;
    bmp_drawrect(B, x1, y1, x2-x1, y2-y1, d);

    // Check base case
    if ((w/3) <= 1 || (h/3) <= 1) return;

    // Recurse on each of the sub-rects.
    //
    // Manually unfolded because it's honestly cleaner.
    recurse_sier_carpet(B, x,  y,  x1-x,  y1-y, d);
    recurse_sier_carpet(B, x1, y,  x2-x1, y1-y, d);
    recurse_sier_carpet(B, x2, y,  x3-x2, y1-y, d);
    recurse_sier_carpet(B, x,  y1, x1-x,  y2-y1, d);
    recurse_sier_carpet(B, x2, y1, x3-x2, y2-y1, d);
    recurse_sier_carpet(B, x,  y2, x1-x,  y3-y2, d);
    recurse_sier_carpet(B, x1, y2, x2-x1, y3-y2, d);
    recurse_sier_carpet(B, x2, y2, x3-x2, y3-y2, d);
}

// Draw a Sierpinski carpet.  Supply top left corner as (x, y) plus width
// and height.
void draw_sier_carpet(BitmapImage* B, unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        DrawFn* d1, DrawFn* d2) {
    // Draw enclosing rect
    bmp_drawrect(B, x, y, w, h, d1);

    // Recurse
    recurse_sier_carpet(B, x, y, w, h, d2);
}

////////////////////////////////////////////////////////////////////////////////
// Sierpinski triangle /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Recursive step for Sierpinski.
//
// Parameters are the enclosing square of the triangle we are drawing an
// upside-down triangle in.
static void recurse_sier_triangle(BitmapImage* B, unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        DrawFn* d) {
    // Calculate corners of this triangle.
    //
    // 1 is the bottom; 2 the top left; 3 the top right.
    unsigned int x1 = x + (w/2);
    unsigned int y1 = y+1;
    unsigned int x2 = x + (w/4);
    unsigned int y2 = y + (h/2);
    unsigned int x3 = x + ((3*w)/4);
    unsigned int y3 = y2;

    // Check base case.  If our triangle is only a single pixel in any
    // dimension, we're done.
    if (x2 >= x3 || y1 == y2) return;

    // Draw contained triangles
    bmp_drawtriangle(B, x1, y1, x2, y2, x3, y3, d);
 
    // Recurse.  Find three bounding rectangles for sub-triangles and draw them.
    recurse_sier_triangle(B, x2, y2, w/2+1, h/2, d); // top
    recurse_sier_triangle(B, x, y, w/2+1, h/2, d); // left
    recurse_sier_triangle(B, x1, y, w/2+1, h/2, d); // right
}

// Draw a Sierpinski's triangle.  Supply top left corner as (x, y) plus width
// and height.
void draw_sier_triangle(BitmapImage* B, unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        DrawFn* d1, DrawFn* d2) {
    // Draw enclosing triangle.
    unsigned int x1 = x + (w/2), y1 = y+h-1;
    unsigned int x2 = x, y2 = y;
    unsigned int x3 = x+w-1, y3 = y;
    bmp_drawtriangle(B, x1, y1, x2, y2, x3, y3, d1);

    // Recurse
    recurse_sier_triangle(B, x, y, w, h, d2);
}

#endif /* _SIERPINSKI_H_ */
