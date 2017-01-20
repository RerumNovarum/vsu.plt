#include <vsu/plt/bmp.h>
#include <vsu/plt/util/alg.h>
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
	return bmp;
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
    ( (0 <= (x) && (x) < (w)) && (0 <= (y) && (y) < (h)) )
#define _VSUPLT_PIXEL(x,y) ( bmp->buf + x + bmp->w*y )

vsuplt_clr
vsuplt_bmp_get(vsuplt_bmp_ptr bmp, int64_t x, int64_t y)
{
    if (!_BOUNDS_VALID(x, y, bmp->w, bmp->h))
        return VSUPLT_BMP_OUT_OF_BOUNDS;
    vsuplt_clr *px = _VSUPLT_PIXEL(x, y);
    return 0x80FFFFFF & *px;
}

void
vsuplt_bmp_set(vsuplt_bmp_ptr bmp, int64_t x, int64_t y, vsuplt_clr clr)
{
    if (_BOUNDS_VALID(x, y, bmp->w, bmp->h)) {
        vsuplt_clr *px = _VSUPLT_PIXEL(x, y);
        *px = clr;
    }
}

#undef _VSUPLT_PIXEL

void
vsuplt_bmp_clear(vsuplt_bmp_ptr bmp, vsuplt_clr color)
{
    for (uint32_t y = 0; y < bmp->h; ++y)
        for (uint32_t x = 0; x < bmp->w; ++x)
            vsuplt_bmp_set(bmp, x, y, color);
}

/* callbacks for /util */

struct _bmp_put_pixel_data
{
    vsuplt_bmp_ptr bmp;
    vsuplt_clr color;
};

void _bmp_put_pixel(void *self, int64_t x, int64_t y)
{
    struct _bmp_put_pixel_data *d = self;
    vsuplt_bmp_set(d->bmp, x, y, d->color);
}

/* end of (callbacks) */

void
vsuplt_bmp_line(vsuplt_bmp_ptr bmp,
        int64_t x0, int64_t y0,
        int64_t x1, int64_t y1,
        vsuplt_clr clr)
{
    struct _bmp_put_pixel_data data;
    data.bmp = bmp;
    data.color = clr;
    vsuplt_bresenhamline(&data, _bmp_put_pixel,
            x0, y0,
            x1, y1);
}

void
vsuplt_bmp_triangle(vsuplt_bmp_ptr bmp,
        int64_t X0, int64_t Y0,
        int64_t X1, int64_t Y1,
        int64_t X2, int64_t Y2,
        vsuplt_clr color)
{
    struct _bmp_put_pixel_data data;
    data.bmp = bmp;
    data.color = color;
    vsuplt_fill_triangle(&data, _bmp_put_pixel,
            X0, Y0, X1, Y1, X2, Y2);
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
