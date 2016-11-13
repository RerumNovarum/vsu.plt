#include <error.h>
#include <stdio.h>
#include <err.h>
#include <vsu/plt.h>

#define _IMG_W (768)
#define _IMG_H (512)
#define _IMG_BUF_SZ (_IMG_W * _IMG_H * VSUPLT_PIXEL_SZ)

#define _MSG_DOESNT_SET_PIXEL \
    "vsuplt_pixel doesn't set the color"
#define _MSG_DOESNT_RESET_PIXEL \
    "vsuplt_init_ctx doesn't reset pixels"

int main()
{
    unsigned char buf[_IMG_BUF_SZ];
    vsuplt_ctx ctx;
    vsuplt_init_ctx(&ctx, buf, _IMG_BUF_SZ, _IMG_W, _IMG_H);

    bool skipcase = false;
    for (int x = 0; x < _IMG_W && !skipcase; ++x)
    {
        for (int y = 0; y < _IMG_H && !skipcase; ++y)
        {
            if (vsuplt_get_px(&ctx, x, y) != 0)
            {
                printf(_MSG_DOESNT_RESET_PIXEL
                        "; (%d %d %X)" "\n",
                        x, y, vsuplt_get_px(&ctx, x, y));
                skipcase = true;
            }
        }
    }

    vsuplt_set_px(&ctx, 0, 0, VSUPLT_RED);
    vsuplt_print_ppm_file(&ctx, "test_vsuplt_pub_api.blank.ppm");
    if (vsuplt_get_px(&ctx, 0, 0) != VSUPLT_RED)
        err(1, _MSG_DOESNT_SET_PIXEL "\n");
    vsuplt_set_px(&ctx, 0, 0, VSUPLT_BLACK);
    if (vsuplt_get_px(&ctx, 0, 0) != VSUPLT_BLACK)
        err(1, _MSG_DOESNT_SET_PIXEL "\n");

    skipcase = false;
    for (int x = 0; x < _IMG_W && !skipcase; ++x)
    {
        int y0 = (1+_IMG_H)/2;
        vsuplt_set_px(&ctx, x, y0, VSUPLT_RED);
        if (vsuplt_get_px(&ctx, x, y0) != VSUPLT_RED)
        {
            printf(_MSG_DOESNT_SET_PIXEL "; (%d %d %X)" "\n", x, y0, VSUPLT_RED);
            skipcase=true;
        }
    }

    vsuplt_print_ppm_file(&ctx, "test_vsuplt_pub_api.ppm");

    vsuplt_clear(&ctx, VSUPLT_WHITE);
    for (int y = 0; y < _IMG_H; ++y)
        for (int x = 0; x < _IMG_W; ++x)
            if (vsuplt_get_px(&ctx, x, y) != VSUPLT_WHITE)
                err(1, "vsuplt_clear doesn't clear");
    int x0 = _IMG_W/2;
    int y0 = _IMG_H/2;
    int a = 128, b = 64;
    vsuplt_line_px(&ctx, x0, y0, x0+a, y0+b, VSUPLT_RED);
    vsuplt_line_px(&ctx, x0, y0, x0-a, y0+b, VSUPLT_BLUE);
    vsuplt_line_px(&ctx, x0, y0, x0+a, y0-b, VSUPLT_BLUE);
    vsuplt_line_px(&ctx, x0, y0, x0-a, y0-b, VSUPLT_RED);
    vsuplt_line_px(&ctx, x0, y0, x0+b, y0+a, VSUPLT_BLACK);
    vsuplt_line_px(&ctx, x0, y0, x0+b, y0-a, VSUPLT_GREEN);
    vsuplt_line_px(&ctx, x0, y0, x0-b, y0-a, VSUPLT_GREEN);
    vsuplt_line_px(&ctx, x0, y0, x0-b, y0+a, VSUPLT_BLACK);
    vsuplt_print_ppm_file(&ctx, "test_vsuplt_line_px.ppm");
    vsuplt_destroy_ctx(&ctx);
    return 0;
}
