#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "bmp.h"
#include "sierpinski.h"

int main(int argc, char* argv[]) {
    // Set up
    int width = 1080;
    int height = 1080;
    BitmapImage* B = bmp_create(width, height);
    DrawFn* d1 = DrawFn_init_axialgradient(0, 0, 0, 1, width, 0,
            0, 255, 255, 255, 0, 255);
    DrawFn* d2 = DrawFn_init_axialgradient(0, 0, 0, 1, width, 0,
            255, 0, 255, 0, 255, 255);

    // Run Sierpinski
    draw_sier_carpet(B, 128, 128, width-256, height-256, d1, d2);

    // Write out Sierpinski and clean up
    bmp_write("sier.bmp", B);
    bmp_free(B);
    DrawFn_free(d1);
    DrawFn_free(d2);
    return 0;
}
