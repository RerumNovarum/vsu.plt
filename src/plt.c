#include <vsu/plt.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

#define _INDEX_OUT_OF_BOUNDS -1
#define BOUNDS_VALID(x, y, w, h) \
    ( (0 <= (x) && (x) <= (w)) && (0 <= (y) && (y) <= (h)) )
#define PIXEL_INDEX(x, y, w, h) \
    ( \
      BOUNDS_VALID((x), (y), (w), (h)) ? \
      (y)*(w) + (x) : \
      _INDEX_OUT_OF_BOUNDS \
    )

/* initialization and disposal */

void
vsuplt_init_ctx(vsuplt_ctx *ctx,
        void *buffer, size_t buff_sz_bytes,
        size_t width, size_t height)
{
    memset(buffer, 0xFF, width*height*VSUPLT_PIXEL_SZ);

    ctx->w = width;
    ctx->h = height;
    ctx->buff = buffer;
    ctx->buff_sz = buff_sz_bytes;

    ctx->must_free_buff = false;
    ctx->must_free_self = false;
}

void
vsuplt_init_ctx_alloc(vsuplt_ctx *ctx,
        size_t width, size_t height)
{
    size_t bs = width * height * VSUPLT_PIXEL_SZ;
    ctx->buff = malloc(bs);
    vsuplt_init_ctx(ctx, ctx->buff, bs, width, height);
    ctx->must_free_buff = true;
}

vsuplt_ctx*
vsuplt_alloc_ctx(size_t width, size_t height)
{
    vsuplt_ctx *ctx = malloc(sizeof(vsuplt_ctx));
    vsuplt_init_ctx_alloc(ctx, width, height);
    ctx->must_free_self = true;
    return ctx;
}

void vsuplt_destroy_ctx(vsuplt_ctx *ctx)
{
    if (ctx->must_free_buff)
    {
        free(ctx->buff);
        ctx->must_free_buff = false;
    }
    if (ctx->must_free_self)
    {
        free(ctx);
    }
}

PIXEL vsuplt_get_px(vsuplt_ctx *ctx, int x, int y)
{
    int index = PIXEL_INDEX(x, y, ctx->w, ctx->h);
    if (index == _INDEX_OUT_OF_BOUNDS)
        return VSUPLT_OUT_OF_BOUNDS;
    return 0x00FFFFFF & ctx->buff[index];
}

int vsuplt_set_px(vsuplt_ctx *ctx, int x, int y, PIXEL clr)
{
    int index = PIXEL_INDEX(x, y, ctx->w, ctx->h);
    if (index == _INDEX_OUT_OF_BOUNDS)
        return VSUPLT_OUT_OF_BOUNDS;
    ctx->buff[index] = clr;
    return 0;
}

void vsuplt_clear(vsuplt_ctx *ctx, PIXEL clr)
{
    int w = ctx->w;
    int h = ctx->h;
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            vsuplt_set_px(ctx, x, y, clr);
}

/* I: (x0, y0), (x1, y1) */
/* I: |y1-y0|<|x1-x0| */
/* I: x0 < x1 */
/* O: line segment from (x0, y0) to (x1, y1) */
/*    of color `clr` drawn on `ctx` */
void
inline static
_vsuplt_line_x(vsuplt_ctx *ctx, int x0, int y0, int x1, int y1, PIXEL clr)
{

    if (x1 < x0)
    {
        long tmp;
        tmp = x0; x0 = x1; x1 = tmp;
        tmp = y0; y0 = y1; y1 = tmp;
    }
    long kx = x1 - x0;
    long ky = y1 - y0;
    long dy = 1;
    if (ky < 0) dy = -1; /* sign(y1-y0) */
    ky = 2*ky*dy; /* 2|y1-y0| */
    kx = -2*kx; /* sign(y1-y0) * 2|x1-x0| */
    long e = ky - (x1 - x0); /* 2H - W */
    vsuplt_set_px(ctx, x0, y0, clr);
    for (long x=x0, y=y0; x <= x1; ++x)
    {
        if (e > 0)
        {
            y += dy;
            e += kx; /* = e_{j-1} -/+ 2W */
        }
        e += ky; /* += 2H */
        vsuplt_set_px(ctx, x, y, clr);
    }
}

void
inline static
_vsuplt_line_y(vsuplt_ctx *ctx, int x0, int y0, int x1, int y1, PIXEL clr)
{

    if (y1 < y0)
    {
        long tmp;
        tmp = x0; x0 = x1; x1 = tmp;
        tmp = y0; y0 = y1; y1 = tmp;
    }
    long kx = x1 - x0;
    long ky = y1 - y0;
    long dx = 1;
    if (kx < 0) dx = -1;
    kx = 2*kx*dx;
    ky = -2*ky;
    long e = kx - (y1 - y0);
    vsuplt_set_px(ctx, x0, y0, clr);
    for (long y=y0, x=x0; y <= y1; ++y)
    {
        if (e > 0)
        {
            x += dx;
            e += ky;
        }
        e += kx;
        vsuplt_set_px(ctx, x, y, clr);
    }
}
void
vsuplt_line(vsuplt_ctx *ctx, int x0, int y0, int x1, int y1, PIXEL clr)
{
    register int W = x1 - x0;
    if (W < 0) W = -W;
    register int H = y1 - y0;
    if (H < 0) H = -H;
    /* octants 1,4,5,8 */
    if (W > H)
    {
        _vsuplt_line_x(ctx, x0, y0, x1, y1, clr);
    }
    else
    {
        _vsuplt_line_y(ctx, x0, y0, x1, y1, clr);
    }
}

/*
 * encoding
 */
#define NETPBM_PPM_BINRASTER "P6"
#define NETPBM_PPM_MAXVAL 255

/* out is expected to be binary */
int vsuplt_print_ppm(vsuplt_ctx *ctx, FILE *out)
{
    size_t w = ctx->w;
    size_t h = ctx->h;
    fprintf(out, NETPBM_PPM_BINRASTER "\n");
    fprintf(out, "%d %d\n", w, h);
    fprintf(out, "%d\n", NETPBM_PPM_MAXVAL);

    flockfile(out);
    for (int y = h-1; y >= 0; --y)
    {
        for (int x = 0; x < w; ++x)
        {
            PIXEL p = vsuplt_get_px(ctx, x, y);
            /*
             *fprintf(
             *        out,
             *        "%d %d %d\n",
             *        VSUPLT_R(p),
             *        VSUPLT_G(p),
             *        VSUPLT_B(p));
             */
            unsigned char pixel[3] = { VSUPLT_R(p), VSUPLT_G(p), VSUPLT_B(p) };
            fwrite_unlocked(pixel, 3, 1, out);
        }
    }
    funlockfile(out);
}
int vsuplt_print_ppm_file(vsuplt_ctx *ctx, char *outfname)
{
    FILE *f = fopen(outfname, "wb");
    if (f == NULL) err(1, "vsuplt_print_ppm: cannot open file %s", outfname);
    vsuplt_print_ppm(ctx, f);
    fclose(f);
}
