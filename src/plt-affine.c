#include <vsu/plt.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* push current CTM to stack */
void vsuplt_save_ctm(vsuplt_ctx *ctx)
{
    vsuplt_ctm *new = malloc(sizeof(vsuplt_ctm));
    if (new == NULL)
        err(VSUPLT_MALLOC_ERROR, "cannot allocate memory");
    memcpy(new, &ctx->ctm, sizeof(vsuplt_ctm));
    ctx->ctm.prev = new;
}

/* pop and use CTM from stack */
void vsuplt_restore_ctm(vsuplt_ctx *ctx)
{
    vsuplt_ctm *prev = ctx->ctm.prev;
    memcpy(&ctx->ctm, prev, sizeof(vsuplt_ctm));
    free(prev);
}

/* translate by a vector with user coordinates (x, y) */
void vsuplt_tr(vsuplt_ctx *ctx, RR x, RR y)
{
    ctx->ctm.T = affine2mul(ctx->ctm.T, affine2tr(x, y));
}

/* rotate about pole by angle phi in user coordinates */
void vsuplt_rot(vsuplt_ctx *ctx, RR phi)
{
    ctx->ctm.T = affine2mul(ctx->ctm.T, affine2rot(phi));
}

/* scale current X axis by x and Y by y */
void vsuplt_scale(vsuplt_ctx *ctx, RR x, RR y)
{
    ctx->ctm.T = affine2mul(ctx->ctm.T, affine2scale(x, y));
}

/* get CTM */
struct affine2
vsuplt_get_affine(vsuplt_ctx *ctx)
{
    return ctx->ctm.T;
}

/* set CTM */
void
vsuplt_set_affine(vsuplt_ctx *ctx, struct affine2 t)
{
    ctx->ctm.T = t;
}

/* line from (x0,y0) to (x1,y1) in user coordinates */
void
vsuplt_line(vsuplt_ctx *ctx, RR x0, RR y0, RR x1, RR y1, PIXEL clr)
{
    affine2rr(ctx->ctm.T, &x0, &y0);
    affine2rr(ctx->ctm.T, &x1, &y1);
    vsuplt_line_px(ctx, x0, y0, x1, y1, clr);
}
