#ifndef _VSUPLT_WIREFRAME2_H_
#define _VSUPLT_WIREFRAME2_H_

#include <vsu/plt/plot2.h>
#include <vsu/num.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>

struct vsuplt_wireframe
{
    size_t V;
    size_t E;
    size_t dim;
    RR *vertices;
    RR *centroid;
    void *edges;
    struct affine2 ctm;
    bool must_free_v;
    bool must_free_e;
    bool must_free_self;
};

typedef struct vsuplt_wireframe * vsuplt_wireframe_ptr;

void
vsuplt_wireframe_init(vsuplt_wireframe_ptr wf);

vsuplt_wireframe_ptr
vsuplt_wireframe_alloc(size_t V, size_t E, size_t dim);

void
vsuplt_wireframe_free(vsuplt_wireframe_ptr wf);

vsuplt_wireframe_ptr
vsuplt_wireframe_load(FILE *in);

vsuplt_wireframe_ptr
vsuplt_wireframe_load_file(char *filename);

void
vsuplt_plot2_draw_wireframe(vsuplt_plot2_ptr plot,
        vsuplt_wireframe_ptr wf);

void
vsuplt_wireframe2_transform(vsuplt_wireframe_ptr wf,
        struct affine2 T);

void
vsuplt_wireframe2_transform_int(vsuplt_wireframe_ptr wf,
        struct affine2 T);

#endif /* ifndef _VSUPLT_WIREFRAME2_H_ */
