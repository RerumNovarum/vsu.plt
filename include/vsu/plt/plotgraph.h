#ifndef _PLOT2_GRAPH_H_
#define _PLOT2_GRAPH_H_

#include <vsu/plt/plot2.h>

struct vsuplt_plotgraph2
{
    /* input */
    vsuplt_clr color, grid_color;
    bool grid;
    void (*transform)(void *cookie, RR *t, RR *u);
    void *cookie;
    RR (*f)(void *cookie, RR t);
    RR t0, t1;
    RR wnd_l, wnd_r, wnd_b, wnd_t;

    /* output */
    RR *x, *y;
    RR xmin, xmax, ymin, ymax;
    NN num_pts;
    void *buf;
};

typedef struct vsuplt_plotgraph2 * vsuplt_plotgraph2_ptr;

vsuplt_plotgraph2_ptr
vsuplt_plotgraph2_alloc(size_t num_pts);

void
vsuplt_plotgraph2_free(vsuplt_plotgraph2_ptr graph);

void
vsuplt_plotgraph2_refill(vsuplt_plotgraph2_ptr graph);

void
vsuplt_plot2_plotgraph(vsuplt_plot2_ptr plot,
    vsuplt_plotgraph2_ptr graph);

#endif /* ifndef _PLOT2_GRAPH_H_ */
