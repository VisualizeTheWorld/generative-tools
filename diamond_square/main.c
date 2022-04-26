#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "diamond_square.h"

int main(int argc, char* argv[]) {
    // Set up
    srand(clock());
    int width = 1080;
    int height = 1080;
    BitmapImage* B = bmp_create(width, height);

    // Run diamond-square
    DrawFn* ds = DrawFn_init_diamondsquare(0, 0, width, height,
            0, 0, 0,
            255, 255, 255);

    // Write diamond-square pattern to image
    bmp_drawrect(B, 0, 0, width, height, ds);

    // Write image and clean up
    bmp_write("ds.bmp", B);
    DrawFn_free(ds);
    bmp_free(B);
    return 0;
}
