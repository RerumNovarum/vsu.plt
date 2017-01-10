#ifndef _VSUPLT_BMP_H_
#define _VSUPLT_BMP_H_

#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

typedef uint32_t vsuplt_clr;

/*
 * pre-defined colors
 */

/* MSB=1 means opaqueness */
#define VSUPLT_COLOR_TRANSPARENT 0x0
#define VSUPLT_COLOR_OPAQUE   0x80000000
#define VSUPLT_COLOR_RED      0x80FF0000
#define VSUPLT_COLOR_GREEN    0x8000FF00
#define VSUPLT_COLOR_BLUE     0x800000FF
#define VSUPLT_COLOR_BLACK    0x80000000
#define VSUPLT_COLOR_WHITE    0x80FFFFFF
#define VSUPLT_COLOR_PURPLE   0x80800080
#define VSUPLT_COLOR_MAGENTA  0x80FF00FF
#define VSUPLT_COLOR_BROWN    0x80A52A2A

#define VSUPLT_RGB(r, g, b) \
    ( (1 << 31)| ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ( b & 0xFF ) )
#define VSUPLT_R(rgb) ( (rgb & 0xFF0000) >> 16 )
#define VSUPLT_G(rgb) ( (rgb & 0xFF00) >> 8 )
#define VSUPLT_B(rgb) ( (rgb & 0xFF) )

struct vsuplt_bmp {
    vsuplt_clr *buf; /* the real bitmap */
    size_t buf_size; /* pixels */
    size_t w, h;
    bool must_free_buf;
    bool must_free_self;
};

typedef struct vsuplt_bmp * vsuplt_bmp_ptr;

void
vsuplt_bmp_init(vsuplt_bmp_ptr bmp,
        void *buf, size_t buf_size_bytes,
        size_t w, size_t h,
        bool free_buf,
        bool free_self);

vsuplt_bmp_ptr
vsuplt_bmp_alloc(size_t w, size_t h);

void
vsuplt_bmp_free(vsuplt_bmp_ptr bmp);

vsuplt_clr
vsuplt_bmp_get(vsuplt_bmp_ptr bmp, int32_t x, int32_t y);

void
vsuplt_bmp_set(vsuplt_bmp_ptr bmp, int32_t x, int32_t y, vsuplt_clr);

void
vsuplt_bmp_line(vsuplt_bmp_ptr bmp,
        int32_t x0, int32_t y0,
        int32_t x1, int32_t y1,
        vsuplt_clr color);

void
vsuplt_bmp_triangle(vsuplt_bmp_ptr bmp,
        int64_t X0, int64_t Y0,
        int64_t X1, int64_t Y1,
        int64_t X2, int64_t Y2,
        vsuplt_clr color);

void
vsuplt_bmp_clear(vsuplt_bmp_ptr bmp, vsuplt_clr color);

/* out is expected to be binary */
int vsuplt_bmp_print_ppm(vsuplt_bmp_ptr bmp, FILE *out);

int vsuplt_bmp_print_ppm_file(vsuplt_bmp_ptr bmp, char *outfname);

/*
 * error codes
 */

#define VSUPLT_BMP_OUT_OF_BOUNDS -1

#endif /* ifndef _VSUPLT_BMP_H_ */
