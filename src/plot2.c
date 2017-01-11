#include <vsu/plt/plot2.h>
#include <string.h>
#include <err.h>

void
vsuplt_plot2_init(vsuplt_plot2_ptr plot,
        void *buf, size_t buf_size,
        size_t buf_width, size_t buf_height,
        RR L, RR R, RR B, RR T,
        bool must_free_buf,
        bool must_free_self)
{
    vsuplt_bmp_init(&plot->bmp,
            buf, buf_size, buf_width, buf_height,
            must_free_buf,
            /* must_free_self */ false);
    plot->ctm.prev = NULL;
    /* NOTE: it'd be more accurate to use complete formulas,
     *       though multiplication seem to be
     *       more symbolic and self-explaining
     */
    plot->ctm.T = affine2mul(
            affine2scale(buf_width/(R-L), 1.0*buf_height/(T-B)),
            plot->ctm.T = affine2tr(-L, -B));

    plot->must_free_self = must_free_self;
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
    vsuplt_plot2_ptr plot = bytes;
    bytes += sizeof(*plot);
    vsuplt_plot2_init(plot, bytes, buf_size, bmp_width, bmp_height,
            L, R, B, T,
            /* must_free_buf */ false,
            /* must_free_self */ true);
	return plot;
}

void
vsuplt_plot2_free(vsuplt_plot2_ptr plot)
{
    while(plot->ctm.prev != NULL)
        vsuplt_plot2_restore_ctm(plot);
    vsuplt_bmp_free(&plot->bmp);
    if (plot->must_free_self)
        free(plot);
}

void
vsuplt_plot2_line(vsuplt_plot2_ptr plot,
        RR x0, RR y0,
        RR x1, RR y1,
        vsuplt_clr color)
{
    affine2apply_rr(plot->ctm.T, &x0, &y0);
    affine2apply_rr(plot->ctm.T, &x1, &y1);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x1, y1, color);
}

/* push current CTM to stack */
void vsuplt_plot2_save_ctm(vsuplt_plot2_ptr plot)
{
    struct vsuplt_ctm2 *new = malloc(sizeof(struct vsuplt_ctm2));
    if (new == NULL)
        err(VSUPLT_MALLOC_ERROR, "cannot allocate memory");
    memcpy(new, &plot->ctm, sizeof(struct vsuplt_ctm2));
    plot->ctm.prev = new;
}

/* pop and use CTM from stack */
void vsuplt_plot2_restore_ctm(vsuplt_plot2_ptr plot)
{
    struct vsuplt_ctm2 *prev = plot->ctm.prev;
    memcpy(&plot->ctm, prev, sizeof(struct vsuplt_ctm2));
    free(prev);
}

/* Add T to the end of the sequence of transformations
 * that will be applied to the point before lighting up the pixel
 */
void vsuplt_plot2_transform(vsuplt_plot2_ptr plot, struct affine2 T)
{
    plot->ctm.T = affine2mul(T, plot->ctm.T);
}

/* Insert T into the begining of the sequence */
void vsuplt_plot2_pretransform(vsuplt_plot2_ptr plot, struct affine2 T)
{
    plot->ctm.T = affine2mul(plot->ctm.T, T);
}

/* Transform "internally", with symmetry relative to the centroid */
void vsuplt_plot2_transform_int(vsuplt_plot2_ptr plot, struct affine2 T)
{
    vsuplt_plot2_pretransform(plot, 
            affine2mul_n(3,
                affine2tr(.5*plot->bmp.w, .5*plot->bmp.h),
                T,
                affine2tr(-.5*plot->bmp.w, -.5*plot->bmp.h)));
}

/* translate by a vector with user coordinates (x, y) */
void vsuplt_plot2_tr(vsuplt_plot2_ptr plot, RR x, RR y)
{
    vsuplt_plot2_transform(plot, affine2tr(x, y));
}

/* rotate about pole by angle phi in user coordinates */
void vsuplt_plot2_rot(vsuplt_plot2_ptr plot, RR phi)
{
    vsuplt_plot2_transform(plot, affine2rot(phi));
}

/* scale current X axis by x and Y by y */
void vsuplt_plot2_scale(vsuplt_plot2_ptr plot, RR x, RR y)
{
    vsuplt_plot2_transform(plot, affine2scale(x, y));
}

/* get CTM */
struct affine2
vsuplt_get_affine(vsuplt_plot2_ptr plot)
{
    return plot->ctm.T;
}

/* set CTM */
void
vsuplt_set_affine(vsuplt_plot2_ptr plot, struct affine2 t)
{
    plot->ctm.T = t;
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
