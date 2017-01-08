#include <vsu/plt/bmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>

void
vsuplt_bmp_init(vsuplt_bmp_ptr bmp,
        void *buf, size_t buf_size_bytes,
        size_t w, size_t h,
        bool free_buf,
        bool free_self)
{
    memset(buf, 0x0, w * h * sizeof(vsuplt_clr));

    bmp->w = w;
    bmp->h = h;
    bmp->buf = buf;
    bmp->buf_size = buf_size_bytes;

    bmp->must_free_buf  = free_buf;
    bmp->must_free_self = free_self;
}

vsuplt_bmp_ptr
vsuplt_bmp_alloc(size_t w, size_t h)
{
    size_t bytes_needed, buf_size;
    void *bytes;
    vsuplt_bmp_ptr bmp;

    buf_size = w * h;
    bytes_needed = sizeof(struct vsuplt_bmp)
                   + sizeof(vsuplt_clr) * buf_size;
    bytes = malloc(bytes_needed);
    if (bytes == NULL) return NULL;
    bmp = bytes;
    bytes += sizeof(struct vsuplt_bmp);

    *bmp = (struct vsuplt_bmp) {
        .buf = bytes,
        .buf_size = buf_size,
        .w = w,
        .h = h,
        .must_free_buf = false,
        .must_free_self = true
    };

    memset(bytes, 0x0, w * h * sizeof(vsuplt_clr));
}

void
vsuplt_bmp_free(vsuplt_bmp_ptr bmp)
{
    if (bmp->must_free_buf) {
        free(bmp->buf);
        bmp->buf = NULL;
    }
    if (bmp->must_free_self) {
        free(bmp);
    }
}

#define _BOUNDS_VALID(x, y, w, h) \
    ( (0 <= (x) && (x) <= (w)) && (0 <= (y) && (y) <= (h)) )
#define _VSUPLT_PIXEL(x,y) (*( bmp->buf + x + bmp->w*y ))

vsuplt_clr
vsuplt_bmp_get(vsuplt_bmp_ptr bmp, int32_t x, int32_t y)
{
    if (!_BOUNDS_VALID(x, y, bmp->w, bmp->h))
        return VSUPLT_BMP_OUT_OF_BOUNDS;
    return 0x80FFFFFF & _VSUPLT_PIXEL(x, y);
}

void
vsuplt_bmp_set(vsuplt_bmp_ptr bmp, int32_t x, int32_t y, vsuplt_clr clr)
{
    if (_BOUNDS_VALID(x, y, bmp->w, bmp->h))
        _VSUPLT_PIXEL(x,y) = clr;
}

#undef _VSUPLT_PIXEL

void
vsuplt_bmp_clear(vsuplt_bmp_ptr bmp, vsuplt_clr color)
{
    for (uint32_t y = 0; y < bmp->h; ++y)
        for (uint32_t x = 0; x < bmp->w; ++x)
            vsuplt_bmp_set(bmp, x, y, color);
}

/*
 * Bresenham line algorithm
 */

/* I: (x0, y0), (x1, y1) */
/* I: |y1-y0|<|x1-x0| */
/* I: x0 < x1 */
/* O: line segment from (x0, y0) to (x1, y1) */
/*    of color `clr` drawn on `bmp` */
void
inline static
_vsuplt_line_x(vsuplt_bmp_ptr bmp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, vsuplt_clr clr)
{
    if (x1 < x0)
    {
        int32_t tmp;
        tmp = x0; x0 = x1; x1 = tmp;
        tmp = y0; y0 = y1; y1 = tmp;
    }
    long kx = x1 - x0;
    long ky = y1 - y0;
    int dy = 1;
    if (ky < 0) dy = -1; /* sign(y1-y0) */
    ky = 2*ky*dy; /* 2|y1-y0| */
    kx = -2*kx; /* sign(y1-y0) * 2|x1-x0| */
    long e = ky - (x1 - x0); /* 2H - W */
    vsuplt_bmp_set(bmp, x0, y0, clr);
    for (long x=x0, y=y0; x <= x1; ++x)
    {
        if (e > 0)
        {
            y += dy;
            e += kx; /* = e_{j-1} -/+ 2W */
        }
        e += ky; /* += 2H */
        vsuplt_bmp_set(bmp, x, y, clr);
    }
}

void
inline static
_vsuplt_line_y(vsuplt_bmp_ptr bmp,
        int x0, int y0,
        int x1, int y1,
        vsuplt_clr clr)
{

    if (y1 < y0)
    {
        uint32_t tmp;
        tmp = x0; x0 = x1; x1 = tmp;
        tmp = y0; y0 = y1; y1 = tmp;
    }
    long kx = x1 - x0;
    long ky = y1 - y0;
    int dx = 1;
    if (kx < 0) dx = -1;
    kx = 2*kx*dx;
    ky = -2*ky;
    long e = kx - (y1 - y0);
    vsuplt_bmp_set(bmp, x0, y0, clr);
    for (long y=y0, x=x0; y <= y1; ++y)
    {
        if (e > 0)
        {
            x += dx;
            e += ky;
        }
        e += kx;
        vsuplt_bmp_set(bmp, x, y, clr);
    }
}

void
vsuplt_bmp_line(vsuplt_bmp_ptr bmp,
        int32_t x0, int32_t y0,
        int32_t x1, int32_t y1,
        vsuplt_clr clr)
{
    register int32_t W = x1 - x0;
    if (W < 0) W = -W;
    register int32_t H = y1 - y0;
    if (H < 0) H = -H;
    if (W > H)
    {
        /* octants 1,4,5,8 */
        _vsuplt_line_x(bmp, x0, y0, x1, y1, clr);
    }
    else
    {
        /* 2,3,6,7 */
        _vsuplt_line_y(bmp, x0, y0, x1, y1, clr);
    }
}

/*
 * encoding
 */
#define NETPBM_PPM_BINRASTER "P6"
#define NETPBM_PPM_MAXVAL 255

/* out is expected to be binary */
int vsuplt_bmp_print_ppm(vsuplt_bmp_ptr bmp, FILE *out)
{
    size_t w = bmp->w;
    size_t h = bmp->h;
    fprintf(out, NETPBM_PPM_BINRASTER "\n");
    fprintf(out, "%d %d\n", w, h);
    fprintf(out, "%d\n", NETPBM_PPM_MAXVAL);

    flockfile(out);
    for (int y = h-1; y >= 0; --y)
    {
        for (int x = 0; x < w; ++x)
        {
            vsuplt_clr p = vsuplt_bmp_get(bmp, x, y);
            unsigned char pixel[3] = { VSUPLT_R(p), VSUPLT_G(p), VSUPLT_B(p) };
            fwrite_unlocked(pixel, 3, 1, out);
        }
    }
    funlockfile(out);
}

int vsuplt_bmp_print_ppm_file(vsuplt_bmp_ptr bmp, char *outfname)
{
    FILE *f = fopen(outfname, "wb");
    if (f == NULL) err(1, "vsuplt_print_ppm: cannot open file %s", outfname);
    vsuplt_bmp_print_ppm(bmp, f);
    fclose(f);
}
