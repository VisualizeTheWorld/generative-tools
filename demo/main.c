#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sierpinski.h"
#include "diamond_square.h"
#include "bmp.h"

// Sierpinski's triangle, drawn out in inverted colors on a cloud fractal
// background.
int demo1() {
    int width = 1920;
    int height = 1920;
    int border = 32;
    BitmapImage* B = bmp_create(width, height);

    // Set up draw functions
    DrawFn* bg = DrawFn_init_diamondsquare(0, 0, width, height,
            32, 32, 2, 224, 224, 224);
    DrawFn* sier1 = DrawFn_init_none();
    DrawFn* sier2 = DrawFn_init_invert();
    
    // Draw a rectangular background
    bmp_drawrect(B, border, border, width-2*border, height-2*border, bg);

    // Write a Sierpinski triangle to the image with two diamond-square
    // patterns.
    draw_sier_triangle(B, 2*border, 2*border, width - 4*border, height - 4*border,
            sier1, sier2);

    // Write image and clean up
    bmp_write("demo1.bmp", B);
    DrawFn_free(sier1);
    DrawFn_free(sier2);
    DrawFn_free(bg);
    bmp_free(B);
}

int main(int argc, char* argv[]) {
    // Set up
    srand(clock());

    // Run demos
    demo1();

    return 0;
}
