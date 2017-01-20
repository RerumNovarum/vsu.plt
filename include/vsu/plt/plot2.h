#ifndef _VSUPLT_PLOT2_H_
#define _VSUPLT_PLOT2_H_

#include <vsu/plt/bmp.h>
#include <vsu/num.h>

struct vsuplt_ctm2
{
    struct affine2 T;
    struct vsuplt_ctm2 *prev;
};

struct vsuplt_plot2
{
    struct vsuplt_bmp bmp;
    struct vsuplt_ctm2 ctm;
    struct affine2 centroid_to_origin;
    struct affine2 origin_to_centroid;
    bool must_free_self;
};

typedef struct vsuplt_plot2 * vsuplt_plot2_ptr;

void
vsuplt_plot2_init(vsuplt_plot2_ptr plot,
        void *buf, size_t buf_size,
        size_t buf_width, size_t buf_height,
        RR L, RR R, RR B, RR T,
        bool must_free_self,
        bool must_free_buf);

vsuplt_plot2_ptr
vsuplt_plot2_alloc(size_t bmp_width, size_t bmp_height,
        RR L, RR R, RR B, RR T);

void
vsuplt_plot2_free(vsuplt_plot2_ptr plot);

void
vsuplt_plot2_set_ctm(vsuplt_plot2_ptr plot, struct affine2 T);

void
vsuplt_plot2_line(vsuplt_plot2_ptr plot,
        RR x0, RR y0,
        RR x1, RR y1,
        vsuplt_clr color);

void
vsuplt_plot2_clear(vsuplt_plot2_ptr plot, vsuplt_clr color);

/* push current CTM to stack */
void vsuplt_plot2_save_ctm(vsuplt_plot2_ptr plot);

/* pop and use CTM from stack */
void vsuplt_plot2_restore_ctm(vsuplt_plot2_ptr plot);

/* Add T to the end of the sequence of transformations
 * that will be applied to the point before lighting up the pixel
 */
void vsuplt_plot2_transform(vsuplt_plot2_ptr plot, struct affine2 T);

/* Insert T into the begining of the sequence */
void vsuplt_plot2_pretransform(vsuplt_plot2_ptr plot, struct affine2 T);

/* Transform "internally", with symmetry relative to the centroid */
void vsuplt_plot2_transform_int(vsuplt_plot2_ptr plot, struct affine2 T);

/* rotate about pole by angle phi in user coordinates */
void vsuplt_plot2_rot(vsuplt_plot2_ptr plot, RR phi);

/* scale current X axis by x and Y by y */
void vsuplt_plot2_scale(vsuplt_plot2_ptr plot, RR x, RR y);

/* translate by a vector with user coordinates (x, y) */
void vsuplt_plot2_tr(vsuplt_plot2_ptr plot, RR x, RR y);

/* get CTM */
struct affine2
vsuplt_get_affine(vsuplt_plot2_ptr plot);

/* set CTM */
void
vsuplt_set_affine(vsuplt_plot2_ptr plot, struct affine2 t);

void
vsuplt_plot2_print_ppm_file(vsuplt_plot2_ptr plot, char *filename);

/* error codes */
#define VSUPLT_MALLOC_ERROR 1

#endif /* ifndef _VSUPLT_PLOT2_H_ */
