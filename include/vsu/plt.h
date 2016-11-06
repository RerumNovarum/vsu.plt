#ifndef _VSUPLT_H_
#define _VSUPLT_H_

#include <stdlib.h>
#include <stdio.h>

/*
 * core structures and interfaces
 */


typedef int32_t PIXEL;
#define VSUPLT_PIXEL_SZ sizeof(PIXEL)
/* TODO: platform-specific */
#define VSUPLT_R(p) ((unsigned char) (((p) & 0x00FF0000) >> 16))
#define VSUPLT_G(p) ((unsigned char) (((p) & 0x0000FF00) >> 8 ))
#define VSUPLT_B(p) ((unsigned char) (((p) & 0x000000FF)      ))

typedef struct vsuplt_ctx vsuplt_ctx;
typedef struct CTM vsuplt_ctm;

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
/*    vsuplt_ctm ctm; */
};
/*
struct vsuplt_ctm
{
    Affine2 T;
    vsuplt_ctm *prev;
}; */

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

PIXEL vsuplt_get_px(vsuplt_ctx *ctx, int x, int y);
int vsuplt_set_px(vsuplt_ctx *ctx, int x, int y, PIXEL rgb);

void vsuplt_clear(vsuplt_ctx *ctx, PIXEL clr);

void
vsuplt_line(vsuplt_ctx *ctx,
        int x0, int y0,
        int x1, int y1,
        PIXEL clr);

void vsuplt_save_ctm(vsuplt_ctx *ctx);
void vsuplt_restore_ctm(vsuplt_ctx *ctx);

/*
 * encoding
 */
int vsuplt_print_ppm(vsuplt_ctx *ctx, FILE *out);
int vsuplt_print_ppm_file(vsuplt_ctx *ctx, char *outfname);

/*
 * constants
 */
#define VSUPLT_OUT_OF_BOUNDS 0x01FFFFFF

/*
 * pre-defined colors
 */

#define VSUPLT_RED 0xFF0000
#define VSUPLT_GREEN 0x00FF00
#define VSUPLT_BLUE 0x0000FF
#define VSUPLT_BLACK 0x000000
#define VSUPLT_WHITE 0xFFFFFF

#endif
