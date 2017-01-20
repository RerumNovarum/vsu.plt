#include <vsu/plt/plotgraph.h>
#include <vsu/plt/backend/fb.h>
#include <stdio.h>
#include <argp.h>
#include <string.h>

#define NUM_PTS 1024
#define CARD_RADIUS 1.0

const char *argp_program_version = "plotgraph 0.01";
const char *argp_program_bug_address = "Sergey Kozlukov <rerumnovarum@openmailbox.org>";

static char doc[] =
"example program that uses plotgraph.h";

static char args_doc[] = "FB_FILENAME";

static struct argp_option options[] = {
    { "mirx", 'm', 0, 0, "Mirror x-coordinate with simmetry to the centroid" },
    { "miry", 'M', 0, 0, "Mirror x-coordinate with simmetry to the centroid" },
    { "rot", 'r', "ANGLE", 0, "Rotate by ANGLE with simmetry to the centroid" },
    { 0 }
};

struct data
{
    vsuplt_plot2_ptr plot;
    vsuplt_plotgraph2_ptr graph;
    void (*handle[256])(vsuplt_plot2_ptr, struct affine2 t);
    struct affine2 t_d[256];
};

RR cardioid(void *self, RR phi)
{
    return 2*CARD_RADIUS*(1 + cosl(phi));
}

void
_vsuplt_transform_polar(void *cookie, RR *phi, RR *rho)
{
    RR x = (*rho) * cosl(*phi);
    RR y = (*rho) * sinl(*phi);
    *phi = x;
    *rho = y;
}

void post_init(vsuplt_fb_ptr fb)
{
    struct data *d = fb->userdata;
    RR dimx = fb->fb_w;
    RR dimy = fb->fb_h;
    if (dimy < dimx) {
        dimx /= dimy;
        dimy = 1;
    } else {
        dimy /= dimx;
        dimx = 1;
    }
    d->plot = vsuplt_plot2_alloc(fb->fb_w, fb->fb_h, -dimx, dimx, -dimy, dimy);
    vsuplt_plot2_clear(d->plot, VSUPLT_COLOR_BLACK);
    fb->bmp = &d->plot->bmp;
    d->graph = vsuplt_plotgraph2_alloc(NUM_PTS);
    d->graph->color = VSUPLT_COLOR_WHITE;
    d->graph->grid_color = VSUPLT_COLOR_GRAY;
    d->graph->grid = true;
    d->graph->transform = _vsuplt_transform_polar;
    d->graph->cookie = NULL;
    d->graph->f = cardioid;
    d->graph->t0 = 0;
    d->graph->t1 = 2*M_PI;
    d->graph->wnd_l = .1;
    d->graph->wnd_r = .9;
    d->graph->wnd_b = .1;
    d->graph->wnd_t = .9;
    vsuplt_plotgraph2_refill(d->graph);
}

void redraw(vsuplt_fb_ptr fb)
{
    struct data *d = fb->userdata;
    vsuplt_plot2_clear(d->plot, VSUPLT_COLOR_BLACK);
    vsuplt_plot2_plotgraph(d->plot, d->graph);
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
    vsuplt_plot2_ptr plot = d->plot;
    if (d->handle[c] != NULL)
        (d->handle[c])(plot, d->t_d[c]);
	vsuplt_fb_redraw_flush(fb);
}

void _key_exit(vsuplt_term_input_ptr incfg, char c)
{
	vsuplt_fb_ptr fb = incfg->self;
    struct data *d = fb->userdata;
    vsuplt_fb_free(fb);
    vsuplt_plot2_free(d->plot);
    vsuplt_plotgraph2_free(d->graph);
    exit(0);
}

struct arguments
{
    char *fb_fname;
    struct affine2 T;
};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *args = state->input;
	switch (key)
	{
		case 'm':
			args->T = affine2mul(args->T, affine2scale(-1,+1));
			break;
		case 'M':
			args->T = affine2mul(args->T, affine2scale(+1,-1));
			break;
		case 'r':
			args->T = affine2mul(args->T, affine2rot(num_sgetr(arg, strlen(arg))));
			break;
		case ARGP_KEY_ARG:
			if (state->arg_num == 0)
				args->fb_fname = arg;
			else argp_usage(state);
			break;
		case ARGP_KEY_NO_ARGS:
			argp_usage(state);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int ret = 0;
int main(int argc, char **argv)
{
    struct arguments arguments = { .T = AFFINE2_ID, .fb_fname = NULL };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    struct vsuplt_fb fb;
    struct data d = { 0 };
    fb.fb_fname = arguments.fb_fname;
    fb.post_init = post_init;
    fb.redraw = redraw;
    fb.userdata = &d;
    for (int i = 0; i < 256; ++i) {
        d.handle[i] = NULL;
    }
    d.t_d['h'] = affine2tr(-DX, 0);
    d.handle['h'] = vsuplt_plot2_transform;
    d.t_d['l'] = affine2tr(+DX, 0);
    d.handle['l'] = vsuplt_plot2_transform;
    d.t_d['j'] = affine2tr(0, -DY);
    d.handle['j'] = vsuplt_plot2_transform;
    d.t_d['k'] = affine2tr(0, +DY);
    d.handle['k'] = vsuplt_plot2_transform;
    d.t_d['+'] = affine2scale(DSCALEX, DSCALEY);
    d.handle['+'] = vsuplt_plot2_transform_int;
    d.t_d['-'] = affine2scale(1.0/DSCALEX, 1.0/DSCALEY);
    d.handle['-'] = vsuplt_plot2_transform_int;
    d.t_d['r'] = affine2rot(DPHI);
    d.handle['r'] = vsuplt_plot2_transform_int;
    d.t_d['R'] = affine2rot(-DPHI);
    d.handle['R'] = vsuplt_plot2_transform_int;
    d.t_d['m'] = affine2scale(+1, -1);
    d.handle['m'] = vsuplt_plot2_transform_int;
    d.t_d['M'] = affine2scale(-1, +1);
    d.handle['M'] = vsuplt_plot2_transform_int;
    for (int i = 0; i < sizeof(fb.in.handlers)/sizeof(*fb.in.handlers); ++i) {
        fb.in.handlers[i] = _on_navig2;
    }
    fb.in.handlers['q'] = _key_exit;
    fb.in.self = &fb;
    vsuplt_fb_show2(&fb);
    return ret;
}
