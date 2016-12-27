#include <vsu/plt.h>
#include <vsu/camera2.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

static void redraw(struct vsuplt_camera2 *cam)
{
    vsuplt_ctx *g = cam->g;

    RR x0 = (cam->L + cam->R)/2;
    RR y0 = (cam->B + cam->T)/2;
    RR rx = cam->R - cam->L;
    RR ry = cam->T - cam->B;
    RR r = sqrt(rx*rx + ry*ry);
    vsuplt_tr(g, x0, y0);
    vsuplt_clear(g, VSUPLT_WHITE);
    vsuplt_save_ctm(g);
    int n = 8; 
    for (int k = 0; k < 2*n; ++k) {
        vsuplt_line(g, 0, 0, r, 0, VSUPLT_RED);
        vsuplt_rot(g, M_PI/n);
    }
    vsuplt_restore_ctm(g);
}

int main()
{
    struct vsuplt_camera2 plot = { 0 };
    plot.L = plot.B = 0;
    plot.R = plot.T = 1;
    vsuplt_camera2_fb_show(&plot);
    return EXIT_SUCCESS;
}
