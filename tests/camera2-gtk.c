#include <vsu/plt.h>
#include <vsu/camera2.h>
#include <stdlib.h>
#include <math.h>

static void redraw(struct vsuplt_camera2 *cam)
{
    vsuplt_ctx *g = cam->g;

    RR x0 = cam->L + .5; /* (cam->L + cam->R)/2; */
    RR y0 = cam->B + .5; /* (cam->B + cam->T)/2; */
    vsuplt_save_ctm(g);
    vsuplt_tr(g, x0, y0);
    vsuplt_clear(g, VSUPLT_WHITE);
    int n = 8; 
    for (int k = 0; k < 2*n; ++k) {
        vsuplt_line(g, 0, 0, 0.2, 0, VSUPLT_RED);
        vsuplt_rot(g, M_PI/n);
    }
    vsuplt_restore_ctm(g);
    vsuplt_line(g, 0.1, 0.1, 0.9, 0.1, VSUPLT_BROWN);
    vsuplt_line(g, 0.9, 0.1, 0.9, 0.9, VSUPLT_BROWN);
    vsuplt_line(g, 0.9, 0.9, 0.1, 0.9, VSUPLT_BROWN);
    vsuplt_line(g, 0.1, 0.9, 0.1, 0.1, VSUPLT_BROWN);
}

int main()
{
    struct vsuplt_camera2 plot = { 0 };
    plot.L = plot.B = 0;
    plot.R = plot.T = 1;
    plot.redraw = redraw;
    vsuplt_camera2_show(&plot);
    return EXIT_SUCCESS;
}
