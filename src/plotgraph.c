#include <vsu/plt/plotgraph.h>
#include <err.h>

vsuplt_plotgraph2_ptr
vsuplt_plotgraph2_alloc(size_t num_pts)
{
    size_t bytes_needed = sizeof(struct vsuplt_plotgraph2) +
        2*sizeof(RR)*num_pts;
    void *buf = malloc(bytes_needed);
    vsuplt_plotgraph2_ptr graph = buf;
    *graph = (struct vsuplt_plotgraph2) { 0 };
    graph->num_pts = num_pts;
    graph->x = buf + sizeof(struct vsuplt_plotgraph2);
    graph->y = graph->x + num_pts;
}

void
vsuplt_plotgraph2_free(vsuplt_plotgraph2_ptr graph)
{
    free(graph->buf);
}

void
vsuplt_plotgraph2_refill(vsuplt_plotgraph2_ptr graph)
{
    RR *x, *y, curt, dt, xmin, xmax, ymin, ymax;
    RR (*f)(void *cookie, RR t);
    void (*transform)(void * cookie, RR* t, RR* u);
    void *cookie;
    size_t num_pts;

    f = graph->f;
    transform = graph->transform;
    cookie = graph->cookie;
    x = graph->x;
    y = graph->y;
    num_pts = graph->num_pts;
    dt = (graph->t1 - graph->t0)/(num_pts - 1);
    curt = graph->t0;
    xmin = ymin = +INFINITY;
    xmax = ymax = -INFINITY;
    for (uint32_t i = 0; i < num_pts; ++i) {
        if (i == num_pts - 1) curt = graph->t1;
        x[i] = curt;
        y[i] = f(cookie, curt);
        if (transform != NULL)
            transform(cookie, x+i, y+i);
        curt += dt;
        if (x[i] < xmin) xmin = x[i];
        else if (x[i] > xmax) xmax = x[i];
        if (y[i] < ymin) ymin = y[i];
        else if (y[i] > ymax) ymax = y[i];
    }
    graph->xmin = xmin;
    graph->xmax = xmax;
    graph->ymin = ymin;
    graph->ymax = ymax;
}

void
vsuplt_plot2_plotgraph(vsuplt_plot2_ptr plot,
    vsuplt_plotgraph2_ptr graphptr)
{
    struct vsuplt_plotgraph2 graph = *graphptr;
    RR *x = graph.x;
    RR *y = graph.y;
    if (graph.num_pts < 2)
        err(1, "vsuplt_plot2_plotgraph: num_pts should at least 2");
    /* TODO: validation */ 

    RR ox, oy;

    vsuplt_plot2_save_ctm(plot);
    ox = graph.wnd_l;
    oy = graph.wnd_b;
    vsuplt_plot2_pretransform(plot, affine2tr(ox, oy));
    vsuplt_plot2_pretransform(plot, affine2scale(
                graph.wnd_r - graph.wnd_l,
                graph.wnd_t - graph.wnd_b
                ));
    if (graph.grid) {
        vsuplt_plot2_line(plot, 0, 0, 0, 1, graph.grid_color);
        vsuplt_plot2_line(plot, 0, 1, 1, 1, graph.grid_color);
        vsuplt_plot2_line(plot, 1, 1, 1, 0, graph.grid_color);
        vsuplt_plot2_line(plot, 1, 0, 0, 0, graph.grid_color);
    }
    vsuplt_plot2_pretransform(plot, affine2tr(.5, .5));
    vsuplt_plot2_pretransform(plot, affine2scale(
                    1.0/(graph.xmax-graph.xmin),
                    1.0/(graph.ymax-graph.ymin))); /* this is so... lispy */
    for (uint32_t i = 1; i < graph.num_pts; ++i) {
        vsuplt_plot2_line(plot, x[i], y[i], x[i-1], y[i-1], graph.color);
    }
    vsuplt_plot2_restore_ctm(plot);
}
