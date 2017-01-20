#include "../src/fb.c"

static int ret = 0;
int main()
{
    struct vsuplt_fb fb;
    fb.fb_fname = "/dev/fb0";
    vsuplt_fb_open(&fb);
    vsuplt_bmp_ptr bmp = vsuplt_bmp_alloc(fb.fb_w, fb.fb_h);
    fb.bmp = bmp;
    vsuplt_bmp_clear(bmp, VSUPLT_COLOR_RED);
    vsuplt_fb_flush(&fb);
    return ret;
}
