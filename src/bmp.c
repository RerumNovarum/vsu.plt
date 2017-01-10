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

struct _bmp_line_put_pixel_data
{
    vsuplt_bmp_ptr bmp;
    vsuplt_clr color;
};

void _bmp_line_put_pixel(void *self, int64_t x, int64_t y)
{
    struct _bmp_line_put_pixel_data *d = self;
    vsuplt_bmp_set(d->bmp, x, y, d->color);
}

void
vsuplt_bmp_line(vsuplt_bmp_ptr bmp,
        int32_t x0, int32_t y0,
        int32_t x1, int32_t y1,
        vsuplt_clr clr)
{
    struct _bmp_line_put_pixel_data data;
    data.bmp = bmp;
    data.color = clr;
    vsuplt_bresenhamline_window(&data, _bmp_line_put_pixel,
            0, bmp->w, 0, bmp->h,
            x0, y0,
            x1, y1);
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
