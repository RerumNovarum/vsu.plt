#include <vsu/num.h>
#include <vsu/plt/plot2.h>
#include <vsu/plt/wireframe.h>
#include <vsu/plt/backend/fb.h>
#include <math.h>

int ret = 0;

struct data
{
    vsuplt_plot2_ptr plot;
    struct affine2 t_d[256];
    void (*handle[256])(vsuplt_plot2_ptr plot, struct affine2 t);
};

void post_init(vsuplt_fb_ptr fb)
{
    struct data *d = fb->userdata;
    d->plot = vsuplt_plot2_alloc(fb->fb_w, fb->fb_h, 0, fb->fb_w, 0, fb->fb_h);
    vsuplt_plot2_clear(d->plot, VSUPLT_COLOR_WHITE);
    fb->bmp = &d->plot->bmp;
}

void redraw(vsuplt_fb_ptr fb)
{
    struct data *d = fb->userdata;
    vsuplt_plot2_ptr plot = d->plot;
    vsuplt_plot2_clear(plot, VSUPLT_COLOR_WHITE);

    RR w = fb->fb_w, h = fb->fb_h;
    RR x0 = .5*w, y0 = .5*h;

    vsuplt_plot2_save_ctm(plot);
    vsuplt_plot2_pretransform(plot, affine2tr(.1*w, .1*h));
    vsuplt_plot2_pretransform(plot, affine2scale(.8*w, .8*h));
    vsuplt_plot2_line(plot, 0, 0, 1, 0, VSUPLT_COLOR_BROWN);
    vsuplt_plot2_line(plot, 1, 0, 1, 1, VSUPLT_COLOR_BROWN);
    vsuplt_plot2_line(plot, 1, 1, 0, 1, VSUPLT_COLOR_BROWN);
    vsuplt_plot2_line(plot, 0, 1, 0, 0, VSUPLT_COLOR_BROWN);
    vsuplt_plot2_line(plot, 0, 0, .5, .5, VSUPLT_COLOR_BLUE);
    vsuplt_plot2_line(plot, .5, .5, 1, 0, VSUPLT_COLOR_MAGENTA);

    vsuplt_plot2_pretransform(plot, affine2tr(.5, .5));
    int n = 8;
    struct affine2 rot = affine2rot(M_PI/n);
    for (int k = 0; k < 2*n; ++k) {
        vsuplt_plot2_pretransform(plot, rot);
        vsuplt_plot2_line(plot, 0, 0, .5, 0, VSUPLT_COLOR_RED);
    }
    vsuplt_plot2_restore_ctm(plot);
}

#define DX 16
#define DY 16
#define DSCALEX 1.2
#define DSCALEY 1.2
#define DPHI 4e-2

void _on_navig2(vsuplt_term_input_ptr incfg, char c)
{
	vsuplt_fb_ptr fb = incfg->self;
    struct data *d = fb->userdata;
    if (d->handle[c] != NULL)
        (d->handle[c])(d->plot, d->t_d[c]);
	vsuplt_fb_redraw_flush(fb);
}

void _key_exit(vsuplt_term_input_ptr incfg, char c)
{
	vsuplt_fb_ptr fb = incfg->self;
    struct data *d = fb->userdata;
    vsuplt_fb_free(fb);
    vsuplt_plot2_free(d->plot);
    exit(0);
}

int main(int argc, char **argv)
{
    struct vsuplt_fb fb;
    struct data d = { 0 };
    fb.fb_fname = argv[1];
    fb.post_init = post_init;
    fb.redraw = redraw;
    fb.userdata = &d;
    for (int i = 0; i < 256; ++i) {
        d.t_d[i] = AFFINE2_ID;
        d.handle[i] = vsuplt_plot2_transform;
    }
    d.t_d['h'] = affine2tr(-DX, 0);
    d.t_d['l'] = affine2tr(+DX, 0);
    d.t_d['j'] = affine2tr(0, -DY);
    d.t_d['k'] = affine2tr(0, +DY);
    d.t_d['+'] = affine2scale(DSCALEX, DSCALEY);
    d.t_d['-'] = affine2scale(1.0/DSCALEX, 1.0/DSCALEY);
    d.t_d['r'] = affine2rot(DPHI);
    d.t_d['R'] = affine2rot(-DPHI);
    d.handle['+'] = vsuplt_plot2_transform_int;
    d.handle['-'] = vsuplt_plot2_transform_int;
    d.handle['r'] = vsuplt_plot2_transform_int;
    d.handle['R'] = vsuplt_plot2_transform_int;
    for (int i = 0; i < sizeof(fb.in.handlers)/sizeof(*fb.in.handlers); ++i) {
        fb.in.handlers[i] = _on_navig2;
    }
    fb.in.handlers['q'] = _key_exit;
    fb.in.self = &fb;
    vsuplt_fb_show2(&fb);
    vsuplt_plot2_free(d.plot);
    return ret;
}

