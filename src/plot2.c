#include <vsu/plt/plot2.h>
#include <string.h>
#include <err.h>

void
vsuplt_plot2_init(vsuplt_plot2_ptr plot2,
        void *buf, size_t buf_size,
        size_t buf_width, size_t buf_height,
        RR L, RR R, RR B, RR T,
        bool must_free_buf,
        bool must_free_self)
{
    vsuplt_bmp_init(&plot2->bmp,
            buf, buf_size, buf_width, buf_height,
            must_free_buf,
            /* must_free_self */ false);
    plot2->ctm.prev = NULL;
    /* NOTE: it'd be more accurate to use complete formulas,
     *       though multiplication seem to be
     *       more symbolic and self-explaining
     */
    plot2->ctm.T = affine2mul(
            affine2scale(buf_width/(R-L), 1.0*buf_height/(T-B)),
            plot2->ctm.T = affine2tr(-L, -B));

    plot2->must_free_self = must_free_self;
}

vsuplt_plot2_ptr
vsuplt_plot2_alloc(size_t bmp_width, size_t bmp_height,
        RR L, RR R, RR B, RR T)
{
    size_t self_size = sizeof(struct vsuplt_plot2);
    size_t buf_size = sizeof(vsuplt_clr) * bmp_width * bmp_height; 
    size_t bytes_needed = self_size + buf_size;
    void *bytes = malloc(bytes_needed);
    if (bytes == NULL) return NULL;
    vsuplt_plot2_ptr plot2 = bytes;
    bytes += sizeof(*plot2);
    vsuplt_plot2_init(plot2, bytes, buf_size, bmp_width, bmp_height,
            L, R, B, T,
            /* must_free_buf */ false,
            /* must_free_self */ true);
}

void
vsuplt_plot2_free(vsuplt_plot2_ptr plot2)
{
    while(plot2->ctm.prev != NULL)
        vsuplt_plot2_restore_ctm(plot2);
    vsuplt_bmp_free(&plot2->bmp);
    if (plot2->must_free_self)
        free(plot2);
}

void
vsuplt_plot2_line(vsuplt_plot2_ptr plot2,
        RR x0, RR y0,
        RR x1, RR y1,
        vsuplt_clr color)
{
    affine2rr(plot2->ctm.T, &x0, &y0);
    affine2rr(plot2->ctm.T, &x1, &y1);
    vsuplt_bmp_line(&plot2->bmp, (uint32_t)x0, (uint32_t)y0, (uint32_t)x1, (uint32_t)y1, color);
}

/* push current CTM to stack */
void vsuplt_plot2_save_ctm(vsuplt_plot2_ptr plot2)
{
    struct vsuplt_ctm2 *new = malloc(sizeof(struct vsuplt_ctm2));
    if (new == NULL)
        err(VSUPLT_MALLOC_ERROR, "cannot allocate memory");
    memcpy(new, &plot2->ctm, sizeof(struct vsuplt_ctm2));
    plot2->ctm.prev = new;
}

/* pop and use CTM from stack */
void vsuplt_plot2_restore_ctm(vsuplt_plot2_ptr plot2)
{
    struct vsuplt_ctm2 *prev = plot2->ctm.prev;
    memcpy(&plot2->ctm, prev, sizeof(struct vsuplt_ctm2));
    free(prev);
}

/* translate by a vector with user coordinates (x, y) */
void vsuplt_plot2_tr(vsuplt_plot2_ptr plot2, RR x, RR y)
{
    plot2->ctm.T = affine2mul(plot2->ctm.T, affine2tr(x, y));
}

/* rotate about pole by angle phi in user coordinates */
void vsuplt_plot2_rot(vsuplt_plot2_ptr plot2, RR phi)
{
    plot2->ctm.T = affine2mul(plot2->ctm.T, affine2rot(phi));
}

/* scale current X axis by x and Y by y */
void vsuplt_plot2_scale(vsuplt_plot2_ptr plot2, RR x, RR y)
{
    plot2->ctm.T = affine2mul(plot2->ctm.T, affine2scale(x, y));
}

/* get CTM */
struct affine2
vsuplt_get_affine(vsuplt_plot2_ptr plot2)
{
    return plot2->ctm.T;
}

/* set CTM */
void
vsuplt_set_affine(vsuplt_plot2_ptr plot2, struct affine2 t)
{
    plot2->ctm.T = t;
}

void
vsuplt_plot2_clear(vsuplt_plot2_ptr plot, vsuplt_clr color)
{
    vsuplt_bmp_clear(&plot->bmp, color);
}

void
vsuplt_plot2_print_ppm_file(vsuplt_plot2_ptr plot, char *filename)
{
    vsuplt_bmp_print_ppm_file(&plot->bmp, filename);
}
