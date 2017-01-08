#include <error.h>
#include <stdio.h>
#include <err.h>
#include <vsu/plt/plot2.h>
#include <math.h>

#define _IMG_W (1536)
#define _IMG_H (1536)

#define _MSG_DOESNT_SET_PIXEL \
    "vsuplt_pixel doesn't set the color"
#define _MSG_DOESNT_RESET_PIXEL \
    "vsuplt_init_plot2 doesn't reset pixels"

int main()
{
    vsuplt_plot2_ptr plot = vsuplt_plot2_alloc(_IMG_W, _IMG_H, 0, _IMG_W, 0, _IMG_H);

    bool skipcase = false;
    for (int x = 0; x < _IMG_W && !skipcase; ++x)
    {
        for (int y = 0; y < _IMG_H && !skipcase; ++y)
        {
            if (vsuplt_bmp_get(&plot->bmp, x, y) != 0)
            {
                printf(_MSG_DOESNT_RESET_PIXEL
                        "; (%d %d %X)" "\n",
                        x, y, vsuplt_bmp_get(&plot->bmp, x, y));
                skipcase = true;
            }
        }
    }

    vsuplt_bmp_set(&plot->bmp, 0, 0, VSUPLT_COLOR_RED);
    vsuplt_plot2_print_ppm_file(plot, "single_pixel.ppm");
    if (vsuplt_bmp_get(&plot->bmp, 0, 0) != VSUPLT_COLOR_RED)
        err(1, _MSG_DOESNT_SET_PIXEL "\n");
    vsuplt_bmp_set(&plot->bmp, 0, 0, VSUPLT_COLOR_BLACK);
    if (vsuplt_bmp_get(&plot->bmp, 0, 0) != VSUPLT_COLOR_BLACK)
        err(1, _MSG_DOESNT_SET_PIXEL "\n");

    skipcase = false;
    for (int x = 0; x < _IMG_W && !skipcase; ++x)
    {
        int y0 = (1+_IMG_H)/2;
        vsuplt_bmp_set(&plot->bmp, x, y0, VSUPLT_COLOR_RED);
        if (vsuplt_bmp_get(&plot->bmp, x, y0) != VSUPLT_COLOR_RED)
        {
            printf(_MSG_DOESNT_SET_PIXEL "; (%d %d %X)" "\n", x, y0, VSUPLT_COLOR_RED);
            skipcase=true;
        }
    }

    vsuplt_plot2_print_ppm_file(plot, "vsuplt_bmp_set.ppm");

    vsuplt_plot2_clear(plot, VSUPLT_COLOR_WHITE);
    for (int y = 0; y < _IMG_H; ++y)
        for (int x = 0; x < _IMG_W; ++x)
            if (vsuplt_bmp_get(&plot->bmp, x, y) != VSUPLT_COLOR_WHITE)
                err(1, "vsuplt_plot2_clear doesn't clear");

    /* affine things */

    int x0 = _IMG_W/2;
    int y0 = _IMG_H/2;
    int a = .25*_IMG_W, b = .3*_IMG_H;
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0+a, y0+b, VSUPLT_COLOR_RED);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0-a, y0+b, VSUPLT_COLOR_BLUE);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0+a, y0-b, VSUPLT_COLOR_BLUE);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0-a, y0-b, VSUPLT_COLOR_RED);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0+b, y0+a, VSUPLT_COLOR_BLACK);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0+b, y0-a, VSUPLT_COLOR_GREEN);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0-b, y0-a, VSUPLT_COLOR_GREEN);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0-b, y0+a, VSUPLT_COLOR_BLACK);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0-a, y0,   VSUPLT_COLOR_PURPLE);
    vsuplt_bmp_line(&plot->bmp, x0, y0, x0+a, y0,   VSUPLT_COLOR_MAGENTA);
    vsuplt_plot2_print_ppm_file(plot, "test_vsuplt_bmp_line.ppm");

    vsuplt_plot2_clear(plot, VSUPLT_COLOR_WHITE);
    vsuplt_plot2_save_ctm(plot);
    vsuplt_plot2_tr(plot, x0, y0);
    int n = 8; 
    for (int k = 0; k < 2*n; ++k) {
        vsuplt_plot2_line(plot, 0, 0, a, 0, VSUPLT_COLOR_RED);
        vsuplt_plot2_rot(plot, M_PI/n);
    }
    vsuplt_plot2_restore_ctm(plot);
    vsuplt_plot2_tr(plot, .1*_IMG_W, .1*_IMG_H);
    vsuplt_plot2_scale(plot, .8*_IMG_W, .8*_IMG_H);
    vsuplt_plot2_line(plot, 0, 0, 1, 0, VSUPLT_COLOR_BROWN);
    vsuplt_plot2_line(plot, 1, 0, 1, 1, VSUPLT_COLOR_BROWN);
    vsuplt_plot2_line(plot, 1, 1, 0, 1, VSUPLT_COLOR_BROWN);
    vsuplt_plot2_line(plot, 0, 1, 0, 0, VSUPLT_COLOR_BROWN);
    vsuplt_plot2_line(plot, 0, 0, .5, .5, VSUPLT_COLOR_RED);
    vsuplt_plot2_line(plot, .5, .5, 1, 0, VSUPLT_COLOR_MAGENTA);
    vsuplt_plot2_print_ppm_file(plot, "test_vsuplt_plot2_line.ppm");

    vsuplt_plot2_free(plot);
    return 0;
}
