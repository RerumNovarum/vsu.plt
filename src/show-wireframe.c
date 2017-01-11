#include <vsu/num.h>
#include <vsu/plt/plot2.h>
#include <vsu/plt/wireframe.h>
#include <vsu/plt/backend/fb.h>
#include <math.h>

/* draws 2dim wireframe model into framebuffer.
 * usage:
 * $ sudo -g video ./wireframe /dev/fb0 model.wf
 * 
 * You should use appropriate video driver.
 * For example, use it in console (tty)
 * or in the X with fbdev driver.
 *
 * navigation:
 *  hjkl translates the model,
 *  r and R rotate in positive and negative direction respectively,
 *  - and + scale.
 * keys are read from terminal in noncanonical mode.
 */

int ret = 0;

struct data
{
    vsuplt_plot2_ptr plot;
    vsuplt_wireframe_ptr wf;
    void (*handle[256])(vsuplt_wireframe_ptr wf, struct affine2 t);
    struct affine2 t_d[256];
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
    vsuplt_plot2_clear(d->plot, VSUPLT_COLOR_WHITE);
    vsuplt_plot2_draw_wireframe(d->plot, d->wf);
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
    vsuplt_wireframe_ptr wf = d->wf;
    if (d->handle[c] != NULL)
        (d->handle[c])(d->wf, d->t_d[c]);
	vsuplt_fb_redraw_flush(fb);
}

void _key_exit(vsuplt_term_input_ptr incfg, char c)
{
	vsuplt_fb_ptr fb = incfg->self;
    struct data *d = fb->userdata;
    vsuplt_fb_free(fb);
    vsuplt_plot2_free(d->plot);
    vsuplt_wireframe_free(d->wf);
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
    FILE *in = fopen(argv[2], "r");
    d.wf = vsuplt_wireframe_load(in);
    for (int i = 0; i < 256; ++i) {
        d.handle[i] = NULL;
    }
    d.t_d['h'] = affine2tr(-DX, 0);
    d.handle['h'] = vsuplt_wireframe2_transform;
    d.t_d['l'] = affine2tr(+DX, 0);
    d.handle['l'] = vsuplt_wireframe2_transform;
    d.t_d['j'] = affine2tr(0, -DY);
    d.handle['j'] = vsuplt_wireframe2_transform;
    d.t_d['k'] = affine2tr(0, +DY);
    d.handle['k'] = vsuplt_wireframe2_transform;
    d.t_d['+'] = affine2scale(DSCALEX, DSCALEY);
    d.handle['+'] = vsuplt_wireframe2_transform_int;
    d.t_d['-'] = affine2scale(1.0/DSCALEX, 1.0/DSCALEY);
    d.handle['-'] = vsuplt_wireframe2_transform_int;
    d.t_d['r'] = affine2rot(DPHI);
    d.handle['r'] = vsuplt_wireframe2_transform_int;
    d.t_d['R'] = affine2rot(-DPHI);
    d.handle['R'] = vsuplt_wireframe2_transform_int;
    for (int i = 0; i < sizeof(fb.in.handlers)/sizeof(*fb.in.handlers); ++i) {
        fb.in.handlers[i] = _on_navig2;
    }
    fb.in.handlers['q'] = _key_exit;
    fb.in.self = &fb;
    vsuplt_fb_show2(&fb);
    vsuplt_plot2_free(d.plot);
    vsuplt_wireframe_free(d.wf);
    return ret;
}
