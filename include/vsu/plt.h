#ifndef _VSUPLT_H_
#define _VSUPLT_H_

#include <stdlib.h>
#include <stdio.h>
#include <vsu/num.h>

/*
 * core structures and interfaces
 */


typedef int32_t PIXEL;
#define VSUPLT_PIXEL_SZ sizeof(PIXEL)
/* TODO: platform-specific */
#define VSUPLT_R(p) ((unsigned char) (((p) & 0x00FF0000) >> 16))
#define VSUPLT_G(p) ((unsigned char) (((p) & 0x0000FF00) >> 8 ))
#define VSUPLT_B(p) ((unsigned char) (((p) & 0x000000FF)      ))
/* VSUPLT_A(p) is 1 if p is opaque */
#define VSUPLT_A(p) ((unsigned char) (((p) & 0x80000000) >> 31))
#define VSUPLT_IS_OPAQUE(p) VSUPLT_A((p))

typedef struct vsuplt_ctx vsuplt_ctx;
typedef struct vsuplt_ctm vsuplt_ctm;

struct vsuplt_ctm
{
    struct affine2 T;
    vsuplt_ctm *prev;
};

/*
 * for compiler to be able to determine size of structure
 * do NOT access struct members directly
 */
struct vsuplt_ctx
{
    size_t w, h;
    PIXEL *buff;
    size_t buff_sz;
    char must_free_buff;
    char must_free_self;
    vsuplt_ctm ctm;
};

/* mainly for stack-allocated buffers */
void
vsuplt_init_ctx(vsuplt_ctx *ctx,
        void *buffer, size_t buff_sz_bytes,
        size_t width, size_t height);

/* snippet for malloc-ing */
void
vsuplt_init_ctx_alloc(vsuplt_ctx *ctx,
        size_t width, size_t height);

void vsuplt_destroy_ctx(vsuplt_ctx *ctx);

/* get pixel (x, y) */
/* these are absolute coordinates (i.e. CTM is not applied) */
/* (0,0) is bottom-left corner */
/* (w-1,h-1) is top-right */
PIXEL vsuplt_get_px(vsuplt_ctx *ctx, int x, int y);
int vsuplt_set_px(vsuplt_ctx *ctx, int x, int y, PIXEL rgb);

/* reset every pixel in the buffer to clr */
void vsuplt_clear(vsuplt_ctx *ctx, PIXEL clr);

/* line segment from pixel (x0, y0) to (x1,y1) */
/* using bresenham algorithm */
void
vsuplt_line_px(vsuplt_ctx *ctx,
        int x0, int y0,
        int x1, int y1,
        PIXEL clr);

/* push current CTM to stack */
void vsuplt_save_ctm(vsuplt_ctx *ctx);

/* pop and use CTM from stack */
void vsuplt_restore_ctm(vsuplt_ctx *ctx);

/* translate by a vector with user coordinates (x, y) */
void vsuplt_tr(vsuplt_ctx *ctx, RR x, RR y);

/* rotate about pole by angle phi in user coordinates */
void vsuplt_rot(vsuplt_ctx *ctx, RR phi);

/* scale current X axis by x and Y by y */
void vsuplt_scale(vsuplt_ctx *ctx, RR x, RR y);

/* get CTM */
struct affine2
vsuplt_get_affine(vsuplt_ctx *ctx);

/* set CTM */
void
vsuplt_set_affine(vsuplt_ctx *ctx, struct affine2 t);

/* line from (x0,y0) to (x1,y1) in user coordinates */
void
vsuplt_line(vsuplt_ctx *ctx, RR x0, RR y0, RR x1, RR y1);

/*
 * encoding
 */
int vsuplt_print_ppm(vsuplt_ctx *ctx, FILE *out);
int vsuplt_print_ppm_file(vsuplt_ctx *ctx, char *outfname);

/*
 * constants
 */

#define VSUPLT_MALLOC_ERROR 1
#define VSUPLT_OUT_OF_BOUNDS 0x01FFFFFF

/*
 * pre-defined colors
 */

/* MSB=1 means opaqueness */
#define VSUPLT_TRANSPARENT 0x0
#define VSUPLT_OPAQUE 0x80000000
#define VSUPLT_RGB(r,g,b)\
    (VSUPLT_OPAQUE | ((r) << 16) | ((g) << 8) | (b))
#define VSUPLT_RED   0x80FF0000
#define VSUPLT_GREEN 0x8000FF00
#define VSUPLT_BLUE  0x800000FF
#define VSUPLT_BLACK 0x80000000
#define VSUPLT_WHITE 0x80FFFFFF

#endif
