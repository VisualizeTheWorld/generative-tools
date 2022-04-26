#ifndef _BMP_BASE_H_
#define _BMP_BASE_H_

#include <stdint.h>

// TODO :
// * Implement bitmap compression scheme(s).

////////////////////////////////////////////////////////////////////////////////
// Bitmap structs //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

typedef struct __attribute__((__packed__)) { // yes, this is bitmap pixel order.
    uint8_t b;
    uint8_t g;
    uint8_t r;
} BitmapPixel;

typedef struct __attribute__((__packed__)) {
    uint8_t sig[2];
    uint32_t size;
    uint16_t reserved[2];
    uint32_t offset;
} BitmapImageHeader;

typedef struct __attribute__((__packed__)) {
    // We only implement the simplest of the headers
    uint32_t header_size;
    uint32_t width;
    uint32_t height;
    uint16_t plane_count;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t data_size;
    uint32_t pixels_per_meter_horiz;
    uint32_t pixels_per_meter_vert;
    uint32_t color_count;
    uint32_t important_color_count;
} BitmapDIBHeader;

typedef struct {
    uint8_t* img;
    int width;
    int height;
    int imgsize;
    BitmapImageHeader* imghdr;
    BitmapDIBHeader* dibhdr;
    BitmapPixel* raw;
} BitmapImage;

////////////////////////////////////////////////////////////////////////////////
// Basic functions /////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Create a new bitmap object in memory.
BitmapImage* bmp_create(int width, int height);

// Write an in-memory bitmap object to a file
void bmp_write(const char* filename, BitmapImage* B);

// Cleans up memory used by a bitmap object
void bmp_free(BitmapImage* B);

// Draws a pixel (specified as R, G, B)
void bmp_drawrgbpixel(BitmapImage* B, unsigned int x, unsigned int y,
        uint8_t r, uint8_t g, uint8_t b);

#endif /* _BMP_BASE_H_ */
