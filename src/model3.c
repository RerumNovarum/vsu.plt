#include <vsu/plt/model3.h>
#include <stdio.h>
#include <err.h>

void
vsuplt_model3_init(vsuplt_model3_ptr pm)
{
}

vsuplt_model3_ptr
vsuplt_model3_alloc(size_t V, size_t E, size_t dim)
{
    vsuplt_model3_ptr pm;
    size_t vertices_bytes_needed = sizeof(*pm->vertices)*dim*V;
    size_t triangles_bytes_needed = (sizeof(uint32_t)*2 + sizeof(vsuplt_clr)) * E;
    size_t centroid_bytes_needed = sizeof(*pm->centroid)*dim;
    size_t self_bytes_needed = sizeof(struct vsuplt_model3);
    size_t bytes_needed = vertices_bytes_needed +
                          triangles_bytes_needed + self_bytes_needed +
                          centroid_bytes_needed;
    void *bytes = malloc(bytes_needed);
    if (bytes == NULL) return NULL;
    pm = bytes;
    bytes += self_bytes_needed;
    void *vertices = bytes;
    bytes += vertices_bytes_needed;
    void *triangles = bytes;
    bytes += triangles_bytes_needed;
    void *centroid = bytes;
    *pm = (struct vsuplt_model3)
            {
                .vertices = vertices,
                .triangles = triangles,
                .centroid = centroid,
                .V = V,
                .E = E,
                .dim = dim,
                .must_free_v = false,
                .must_free_e = false,
                .must_free_self = true,
                .ctm = AFFINE2_ID
            };
    return pm;
}

void
vsuplt_model3_free(vsuplt_model3_ptr pm)
{
    if (pm->must_free_v) {
        free(pm->vertices);
        pm->vertices = NULL;
    }
    if (pm->must_free_e) {
        free(pm->triangles);
        pm->triangles = NULL;
    }
    if (pm->must_free_self)
        free(pm);
}

vsuplt_model3_ptr
vsuplt_model3_load(FILE *in)
{
    size_t V, P;
    flockfile(in);
    fscanf(in, "%zu%zu", &V, &E);
    vsuplt_model3_ptr pm;
    pm = vsuplt_model3_alloc(V, P);
    void *elt = pm->vertices;
    for (uint32_t v = 0; v < V; ++v) {
        for (int i = 0; i < dim; ++i) {
            long double x;
            fscanf(in, "%Lf", &x);
            pm->centroid[i%pm->dim] += x/V;
            *(long double *)elt = x;
            elt += sizeof(x);
        }
    }
    elt = pm->triangles;
    for (uint32_t p = 0; p < P; ++p) {
        unsigned int u, v, w, clr;
        fscanf(in, "%u%u%u%x", &u, &v, &w, &clr);
        *(uint32_t*)elt = u;
        elt += sizeof(uint32_t);
        *(uint32_t*)elt = v;
        elt += sizeof(uint32_t);
        *(uint32_t*)elt = w;
        elt += sizeof(uint32_t);
        *(vsuplt_clr*)elt = clr;
        elt += sizeof(vsuplt_clr);
    }
    funlockfile(in);
    return pm;
}

vsuplt_model3_ptr
vusplt_model3_load_file(char *filename)
{
    FILE *in = fopen(filename, "r");
    vsuplt_model3_ptr pm;
    pm = vsuplt_model3_load(in);
    fclose(in);
    return pm;
}

vsuplt_model3_triangle
vsuplt_model3_getmesh_unmod(vsuplt_model3_ptr model, uint32_t p)
{
    uint32_t *mptr = model->triangles + (sizeof(vsuplt_clr) + 3*sizeof(uint32_t))*p;
    struct vsuplt_model3_triangle mesh = {
        .u = *mptr,
        .v = *(mptr+1),
        .w = *(mptr+2),
        .color = *(vsuplt_clr*)(mptr+3)
    };
    return mesh;
}

vsuplt_model3_vertice
vsuplt_model3_getvert_unmod(vsuplt_model3_ptr model, uint32_t v)
{
    RR *vptr = model->vertices + 3*sizeof(RR)*v;
    struct vsuplt_model3_vertice vert = {
        .x = *vptr,
        .y = *(vptr+1),
        .z = *(vptr+2)
    };
    return vert;
}

vsuplt_model3_vertice
vsuplt_model3_getvert(vsuplt_model3_ptr model, uint32_t v)
{
    vsuplt_model3_triangle vert = vsuplt_model3_getvert_umod(model, p);
    affine3rr(model->ctm, &vert.x, &vert.y, &vert.z);
    return vert;
}

void
vsuplt_model3_transform(vsuplt_model3_ptr pm,
        struct affine3 T)
{
    pm->ctm = affine3mul(pm->ctm, T);
    affine3rr(T, pm->centroid, pm->centroid + 1);
}

void
vsuplt_model3_pretransform(vsuplt_model3_ptr pm,
        struct affine3 T)
{
    pm->ctm = affine3mul(pm->ctm, T);
}

void
vsuplt_model3_transform_int(vsuplt_model3_ptr pm,
        struct affine3 T)
{
    struct affine3 tr_inv = affine3tr(pm->centroid[0], pm->centroid[1], pm->centroid[2]);
    struct affine3 tr = tr_inv;
    tr.b1 *= -1;
    tr.b2 *= -1;
    tr.b3 *= -1;
    pm->ctm = affine3mul_n(4,
            pm->ctm,
            tr_inv,
            T,
            tr);
}
