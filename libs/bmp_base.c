#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "bmp_base.h"

// Every C file needs some idiosyntratic #defines
#define PAD_TO(size, align) ((((((size)-1) / align)+1) * align))
#define PTR_BYTE_ADD(p, x) ((void*)(((char*)(p))+(x)))
#define SWAP(a, b) { typeof(a) tmp = a; a = b; b = tmp; }
#define ROWWIDTH(width) (PAD_TO((width)*sizeof(BitmapPixel), 4))

////////////////////////////////////////////////////////////////////////////////
// Bitmap basics ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

_Static_assert(sizeof(BitmapImageHeader) == 14);
_Static_assert(sizeof(BitmapDIBHeader) == 40);
_Static_assert(sizeof(BitmapPixel) == 3);

BitmapImage* bmp_create(int width, int height) {
    // Calculate size
    unsigned int raw_row_width = ROWWIDTH(width);
    unsigned int raw_data_size = raw_row_width * height;
    unsigned int size = 0;
    size += sizeof(BitmapImageHeader);
    size += sizeof(BitmapDIBHeader);
    size += raw_data_size;
    
    // Set up the struct.
    BitmapImage* B = calloc(sizeof(BitmapImage), 1);
    B->width = width;
    B->height = height;
    B->imgsize = size;
    B->img = malloc(size);
    B->imghdr = (BitmapImageHeader*)(B->img);
    B->dibhdr = (BitmapDIBHeader*)PTR_BYTE_ADD((B->img), sizeof(BitmapImageHeader));
    B->raw = (BitmapPixel*)PTR_BYTE_ADD((B->img), sizeof(BitmapImageHeader) + sizeof(BitmapDIBHeader));

    // Set up the image header.
    B->imghdr->sig[0] = 0x42;
    B->imghdr->sig[1] = 0x4d;
    B->imghdr->size = size;
    B->imghdr->reserved[0] = 0;
    B->imghdr->reserved[1] = 0;
    B->imghdr->offset = sizeof(BitmapImageHeader) + sizeof(BitmapDIBHeader);

    // Set up the DIB header.
    B->dibhdr->header_size = sizeof(BitmapDIBHeader);
    B->dibhdr->width = width;
    B->dibhdr->height = height;
    B->dibhdr->plane_count = 1;
    B->dibhdr->bits_per_pixel = sizeof(BitmapPixel) * 8;
    B->dibhdr->compression = 0; // no compression
    B->dibhdr->data_size = raw_data_size;
    B->dibhdr->pixels_per_meter_horiz = 2835; // 72 dpi
    B->dibhdr->pixels_per_meter_vert = 2835; // 72 dpi
    B->dibhdr->color_count = 0;
    B->dibhdr->important_color_count = 0;

    // Color the background black.
    memset(B->raw, 0, raw_data_size);

    return B;
}

void bmp_write(const char* filename, BitmapImage* B) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH);
    assert(fd >= 0);
    assert(write(fd, B->img, B->imgsize) == B->imgsize);
    assert(close(fd) == 0);
}

void bmp_free(BitmapImage* B) {
    free(B->img);
    free(B);
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

void bmp_drawrgbpixel(BitmapImage* B, unsigned int x, unsigned int y,
        uint8_t r, uint8_t g, uint8_t b) {
    internal_drawrgbpixel(B, x, y, r, g, b);
}
