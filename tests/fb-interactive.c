#include <vsu/num.h>
#include <vsu/plt/plot2.h>
#include <vsu/plt/wireframe.h>
#include <vsu/plt/backend/fb.h>
#include <math.h>

#define IMG_W 1024
#define IMG_H 1024

int ret = 0;

char *wf_encoded = "42 43 2\n"
/* vertices */
/* 0 */  "176 530\n" "360 530\n" "390 450\n" "420 530\n"
/* 4 */  "540 530\n" "440 220\n" "350 220\n" "300 410\n"
/* 8 */  "350 410\n" "390 330\n" "340 330\n" "640 530\n"
/* 12 */ "645 410\n" "650 440\n" "660 410\n" "660 530\n"
/* 16 */ "760 530\n" "760 235\n" "735 220\n" "675 220\n"
/* 20 */ "650 240\n" "650 270\n" "640 220\n" "570 220\n"
/* 24 */ "560 230\n" "560 220\n" "500 220\n" "501 406\n"
/* 28 */ "890 530\n" "890 430\n" "895 445\n" "900 420\n"
/* 32 */ "900 520\n" "990 520\n" "990 235\n" "965 220\n"
/* 36 */ "890 220\n" "865 230\n" "855 220\n" "790 220\n"
/* 40 */ "770 230\n" "770 220\n"
/* edges */
/* 0 */  "0 1 0xFF0000\n"   "1 2 0xFF0000\n"    "2 3 0xFF0000\n"   "3 4 0xFF0000\n"
/* 4 */  "4 5 0xFF0000\n"   "5 6 0xFF0000\n"    "6 0 0xFF0000\n"   "7 8 0xFF0000\n"
/* 8 */  "8 9 0xFF0000\n"   "9 10 0xFF0000\n"   "11 12 0xFF0000\n" "13 14 0xFF0000\n"
/* 12 */ "15 16 0xFF0000\n" "16 17 0xFF0000\n"  "17 18 0xFF0000\n" "18 19 0xFF0000\n"
/* 16 */ "19 20 0xFF0000\n" "20 21 0xFF0000\n"  "20 22 0xFF0000\n" "22 23 0xFF0000\n"
/* 20 */ "23 24 0xFF0000\n" "24 25 0xFF0000\n"  "25 26 0xFF0000\n" "26 27 0xFF0000\n"
/* 24 */ "16 28 0xFF0000\n" "28 29 0xFF0000\n"  "29 30 0xFF0000\n" "30 31 0xFF0000\n"
/* 28 */ "31 32 0xFF0000\n" "33 34 0xFF0000\n"  "34 35 0xFF0000\n" "36 37 0xFF0000\n"
/* 32 */ "38 39 0xFF0000\n" "39 40 0xFF0000\n"  "40 41 0xFF0000\n" "41 18 0xFF0000\n"
/* breaking the order: */
"10 7 0xFF0000\n" "4 11 0xFF0000\n" "12 13 0xFF0000\n" "14 15 0xFF0000\n"
"32 33 0xFF0000\n" "35 36 0xFF0000\n" "37 38 0xFF0000\n";

struct data
{
    vsuplt_plot2_ptr plot;
    vsuplt_wireframe_ptr wf;
    struct affine2 t_dphi;
    struct affine2 t_dphi_inv;
};

void post_init(vsuplt_fb_ptr fb)
{
    struct data *d = fb->userdata;
    d->plot = vsuplt_plot2_alloc(fb->fb_w, fb->fb_h, 0, fb->fb_w, 0, fb->fb_h);
    vsuplt_plot2_clear(d->plot, VSUPLT_COLOR_WHITE);
    fb->bmp = &d->plot->bmp;
    vsuplt_wireframe2_transform(d->wf, affine2mul_n(2,
                affine2tr(0, fb->fb_h),
                affine2scale(1, -1)));
}

void redraw(vsuplt_fb_ptr fb)
{
    struct data *d = fb->userdata;
    vsuplt_plot2_clear(d->plot, VSUPLT_COLOR_WHITE);
    vsuplt_plot2_draw_wireframe(d->plot, d->wf);
}

#define DX 8
#define DY 8
#define DSCALEX 1.2
#define DSCALEY 1.2
#define DPHI 1e-2

void _on_navig2(vsuplt_term_input_ptr incfg, char c)
{
	vsuplt_fb_ptr fb = incfg->self;
    struct data *d = fb->userdata;
    vsuplt_wireframe_ptr wf = d->wf;
	switch (c)
	{
        case 'l':
            vsuplt_wireframe2_transform(wf, affine2tr(DX, 0));
            break;
        case 'h':
            vsuplt_wireframe2_transform(wf, affine2tr(-DX, 0));
            break;
        case 'j':
            vsuplt_wireframe2_transform(wf, affine2tr(0, -DY));
            break;
        case 'k':
            vsuplt_wireframe2_transform(wf, affine2tr(0, DY));
            break;
        case '+':
            /* vsuplt_wireframe2_transform_int
             * translates the origin of coordinates to the centroid of wireframe
             * applies T
             * and translates back.
             * 
             * we use it for scaling and rotation.
             */
            vsuplt_wireframe2_transform_int(wf, affine2scale(DSCALEX, DSCALEY));
            break;
        case '-':
            vsuplt_wireframe2_transform_int(wf, affine2scale(1.0/DSCALEX, 1.0/DSCALEY));
            break;
        case 'r':
            vsuplt_wireframe2_transform_int(wf, d->t_dphi);
            break;
        case 'R':
            vsuplt_wireframe2_transform_int(wf, d->t_dphi_inv);
            break;

	}
	vsuplt_fb_redraw_flush(fb);
}

int main()
{
    struct vsuplt_fb fb;
    struct data d;
    fb.fb_fname = "/dev/fb0";
    fb.post_init = post_init;
    fb.redraw = redraw;
    fb.userdata = &d;
    FILE *in = fmemopen(wf_encoded, strlen(wf_encoded), "r");
    d.wf = vsuplt_wireframe_load(in);
    d.t_dphi = affine2rot(DPHI);
    d.t_dphi_inv = affine2rot(-DPHI);
    for (int i = 0; i < sizeof(fb.in.handlers)/sizeof(*fb.in.handlers); ++i) {
        fb.in.handlers[i] = _on_navig2;
    }
    fb.in.self = &fb;
    vsuplt_fb_show2(&fb);
    vsuplt_plot2_free(d.plot);
    vsuplt_wireframe_free(d.wf);
    return ret;
}
