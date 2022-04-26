#ifndef _SIERPINSKI_H_
#define _SIERPINSKI_H_

#include <stdint.h>
#include "bmp.h"

void draw_sier_triangle(BitmapImage* B,
        unsigned int x, unsigned int y, unsigned int w, unsigned int h,
        DrawFn* d1, DrawFn* d2);

void draw_sier_carpet(BitmapImage* B,
        unsigned int x, unsigned int y, unsigned int w, unsigned int h,
        DrawFn* d1, DrawFn* d2);

#endif /* _SIERPINSKI_H_ */
