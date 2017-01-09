#include <vsu/plt/wireframe.h>
#include <stdio.h>
#include <err.h>

void
vsuplt_wireframe_init(vsuplt_wireframe_ptr wf)
{
}

vsuplt_wireframe_ptr
vsuplt_wireframe_alloc(size_t V, size_t E, size_t dim)
{
    vsuplt_wireframe_ptr wf;
    size_t vertices_bytes_needed = sizeof(*wf->vertices)*dim*V;
    size_t edges_bytes_needed = (sizeof(uint32_t)*2 + sizeof(vsuplt_clr)) * E;
    size_t centroid_bytes_needed = sizeof(*wf->centroid)*dim;
    size_t self_bytes_needed = sizeof(struct vsuplt_wireframe);
    size_t bytes_needed = vertices_bytes_needed +
                          edges_bytes_needed + self_bytes_needed +
                          centroid_bytes_needed;
    void *bytes = malloc(bytes_needed);
    if (bytes == NULL) return NULL;
    wf = bytes;
    bytes += self_bytes_needed;
    void *vertices = bytes;
    bytes += vertices_bytes_needed;
    void *edges = bytes;
    bytes += edges_bytes_needed;
    void *centroid = bytes;
    *wf = (struct vsuplt_wireframe)
            {
                .vertices = vertices,
                .edges = edges,
                .centroid = centroid,
                .V = V,
                .E = E,
                .dim = dim,
                .must_free_v = false,
                .must_free_e = false,
                .must_free_self = true,
                .ctm = AFFINE2_ID
            };
    return wf;
}

void
vsuplt_wireframe_free(vsuplt_wireframe_ptr wf)
{
    if (wf->must_free_v) {
        free(wf->vertices);
        wf->vertices = NULL;
    }
    if (wf->must_free_e) {
        free(wf->edges);
        wf->edges = NULL;
    }
    if (wf->must_free_self)
        free(wf);
}

vsuplt_wireframe_ptr
vsuplt_wireframe_load(FILE *in)
{
    size_t V, E, dim;
    flockfile(in);
    fscanf(in, "%zu%zu%zu", &V, &E, &dim);
    vsuplt_wireframe_ptr wf;
    wf = vsuplt_wireframe_alloc(V, E, dim);
    void *elt = wf->vertices;
    for (uint32_t v = 0; v < V; ++v) {
        for (int i = 0; i < dim; ++i) {
            long double x;
            fscanf(in, "%Lf", &x);
            wf->centroid[i%wf->dim] += x/V;
            *(long double *)elt = x;
            elt += sizeof(x);
        }
    }
    elt = wf->edges;
    for (uint32_t e = 0; e < E; ++e) {
        unsigned int i, j, clr;
        fscanf(in, "%u%u%x", &i, &j, &clr);
        *(uint32_t*)elt = i;
        elt += sizeof(uint32_t);
        *(uint32_t*)elt = j;
        elt += sizeof(uint32_t);
        *(vsuplt_clr*)elt = clr;
        elt += sizeof(vsuplt_clr);
    }
    funlockfile(in);
    return wf;
}

vsuplt_wireframe_ptr
vusplt_wireframe_load_file(char *filename)
{
    FILE *in = fopen(filename, "r");
    vsuplt_wireframe_ptr wf;
    wf = vsuplt_wireframe_load(in);
    fclose(in);
    return wf;
}

void
vsuplt_wireframe2_transform(vsuplt_wireframe_ptr wf,
        struct affine2 T)
{
    wf->ctm = affine2mul(T, wf->ctm);
    affine2rr(T, wf->centroid, wf->centroid + 1);
}

void
vsuplt_wireframe2_transform_int(vsuplt_wireframe_ptr wf,
        struct affine2 T)
{
    struct affine2 tr_inv = affine2tr(wf->centroid[0], wf->centroid[1]);
    struct affine2 tr = tr_inv;
    tr.b1 *= -1;
    tr.b2 *= -1;
    wf->ctm = affine2mul_n(4,
            tr_inv,
            T,
            tr,
            wf->ctm);
}
static inline uint32_t
_wf_i(vsuplt_wireframe_ptr wf, uint32_t e) {
    return *(uint32_t*)(wf->edges + (2*sizeof(uint32_t) + sizeof(vsuplt_clr))*e);
}

static inline uint32_t
_wf_j(vsuplt_wireframe_ptr wf, uint32_t e) {
    return *(uint32_t*)(wf->edges +
            (2*sizeof(uint32_t) + sizeof(vsuplt_clr))*e +
             sizeof(uint32_t));
}

static inline vsuplt_clr
_wf_clr(vsuplt_wireframe_ptr wf, uint32_t e) {
    return *(vsuplt_clr*)(wf->edges +
             (2*sizeof(uint32_t) + sizeof(vsuplt_clr))*e +
              sizeof(uint32_t) +
              sizeof(vsuplt_clr));
}

static inline RR
_wf_x(vsuplt_wireframe_ptr wf, uint32_t v) {
    return *(RR*)(wf->vertices + wf->dim * v);
}

static inline RR
_wf_y(vsuplt_wireframe_ptr wf, uint32_t v) {
    return *(RR*)(wf->vertices + wf->dim * v + 1);
}

void
vsuplt_plot2_draw_wireframe(vsuplt_plot2_ptr plot,
        vsuplt_wireframe_ptr wf)
{
    if (wf->dim != 2)
        err(1, "vsuplt_plot2_draw_wireframe: wrong wf dimension");
    for (uint32_t e = 0; e < wf->E; ++e) {
        uint32_t i = _wf_i(wf, e);
        uint32_t j = _wf_j(wf, e);
        vsuplt_clr clr = _wf_clr(wf, e);
        RR x0 = _wf_x(wf, i), y0 = _wf_y(wf, i);
        RR x1 = _wf_x(wf, j), y1 = _wf_y(wf, j);
        affine2rr(wf->ctm, &x0, &y0); 
        affine2rr(wf->ctm, &x1, &y1); 
        vsuplt_plot2_line(plot, x0, y0, x1, y1, clr);
    }
}
