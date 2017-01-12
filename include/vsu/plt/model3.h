#ifndef _VSUPLT_WIREFRAME2_H_
#define _VSUPLT_WIREFRAME2_H_

#include <vsu/plt/plot2.h>
#include <vsu/num.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>

struct vsuplt_model3_triangle
{
    uint32_t u, v, w;
    vsuplt_clr color;
};

struct vsuplt_model3_vertice
{
    RR x, y, z;
};

struct vsuplt_model3
{
    size_t V;
    size_t P;
    RR *vertices;
    RR *centroid;
    vsuplt_model3_triangle *triangles;
    struct affine3 ctm;
    bool must_free_v;
    bool must_free_p;
    bool must_free_self;
};

typedef struct vsuplt_model3 * vsuplt_model3_ptr;

void
vsuplt_model3_init(vsuplt_model3_ptr pm);

vsuplt_model3_ptr
vsuplt_model3_alloc(size_t V, size_t E, size_t dim);

void
vsuplt_model3_free(vsuplt_model3_ptr pm);

vsuplt_model3_ptr
vsuplt_model3_load(FILE *in);

vsuplt_model3_ptr
vsuplt_model3_load_file(char *filename);

/* TODO: vsuplt_model3_load_obj */

vsuplt_model3_triangle
vsuplt_model3_getmesh(vsuplt_model3_ptr model, uint32_t p);

/* get unmodified vertice coordinates
 * just as in input
 */
vsuplt_model3_vertice
vsuplt_model3_getvert_unmod(vsuplt_model3_ptr model, uint32_t v);

vsuplt_model3_vertice
vsuplt_model3_getvert(vsuplt_model3_ptr model, uint32_t v);

void
vsuplt_model3_transform(vsuplt_model3_ptr pm,
        struct affine3 T);

void
vsuplt_model3_pretransform(vsuplt_model3_ptr pm,
        struct affine3 T);

void
vsuplt_model3_transform_int(vsuplt_model3_ptr pm,
        struct affine3 T);

#endif /* ifndef _VSUPLT_WIREFRAME2_H_ */

