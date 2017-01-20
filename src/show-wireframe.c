#include <vsu/num.h>
#include <vsu/plt/plot2.h>
#include <vsu/plt/wireframe.h>
#include <vsu/plt/backend/fb.h>
#include <math.h>
#include <argp.h>

const char *argp_program_version =
    "wireframe 0.1";
const char *argp_program_bug_address =
    "Sergey Kozlukov <rerumnovarum@openmailbox.org>";

static char doc[] =
    "Draw two-dimensional wireframe model into "
    "linux framebuffer (see [linux/fb.h](/usr/include/linux/fb.h))\n"
    "Example usage:\n"
    "  $ sudo -g video ./wireframe /dev/fb0 model.wf\n"
	"Wireframe model file structure is as follows:\n"
	"  V E dimensions\n"
	"  x_1 y_1\n"
	"    ...\n"
	"  x_V y_V\n"
	"  e_1.v e_1.w e_1.color\n"
	"    ...\n"
	"  e_E.v e_E.w e_E.color\n"
	"where V is the number of vertices, "
	"E is the number of edges, "
	"dim=2 is the number of dimensions, "
	"(x_j,y_j) are the coordinates of j'th vertice, "
	"e_j.v and e_j.w are the serial numbers of vertices incident to the j'th edge, "
	"and the e_j.color is its color.\n"
    "You should use appropriate video driver. "
    "For example, use it in console (tty) "
    "or in the X with fbdev driver.\n"
    "navigation:\n"
    "  hjkl translates the model,\n"
    "  r and R rotate in positive and negative direction respectively,\n"
    "  - and + scale,\n"
    "  m and M mirror x- and y-coordinate respectively,\n"
    "Scalings, rotations and mirrorings are applied with simmetry to the centroid, "
    "which is calculated as the expectation (i.e. the average) of the coordinates of vertices. "
    "That means, we translate centroid to the origin, apply transformation and shift back.\n"
    "Keys are read from terminal in noncanonical mode.";

static char args_doc[] = "FB_FILENAME MODEL_FILENAME";

static struct argp_option options[] = {
    { "mirx", 'm', 0, 0, "Mirror x-coordinate with simmetry to the centroid" },
    { "miry", 'M', 0, 0, "Mirror x-coordinate with simmetry to the centroid" },
    { "rot", 'r', "ANGLE", 0, "Rotate by ANGLE with simmetry to the centroid" },
    { 0 }
};

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

struct arguments
{
    char *fb_fname, *wf_fname;
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
			else if (state->arg_num == 1)
				args->wf_fname = arg;
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

int main(int argc, char **argv)
{
    struct arguments arguments = { .T = AFFINE2_ID, .fb_fname = NULL, .wf_fname = NULL };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    struct vsuplt_fb fb;
    struct data d = { 0 };
    fb.fb_fname = arguments.fb_fname;
    fb.post_init = post_init;
    fb.redraw = redraw;
    fb.userdata = &d;
    d.wf = vsuplt_wireframe_load_file(arguments.wf_fname);
	vsuplt_wireframe2_transform_int(d.wf, arguments.T);
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
    d.t_d['m'] = affine2scale(+1, -1);
    d.handle['m'] = vsuplt_wireframe2_transform_int;
    d.t_d['M'] = affine2scale(-1, +1);
    d.handle['M'] = vsuplt_wireframe2_transform_int;
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
