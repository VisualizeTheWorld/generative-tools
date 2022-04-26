#ifndef _DIAMONDSQUARE_H_
#define _DIAMONDSQUARE_H_

#include "bmp.h"

// Draw a diamond-square algorithm (cloud fractal) pattern.  Specify the area to
// precompute a DS for as a top left + w, h rectangle.  Behavior may be
// unexpected outside this area.
DrawFn* DrawFn_init_diamondsquare(int x, int y, int w, int h,
        uint8_t r1, uint8_t g1, uint8_t b1,
        uint8_t r2, uint8_t g2, uint8_t b2);

#endif /* _DIAMONDSQUARE_H_ */
