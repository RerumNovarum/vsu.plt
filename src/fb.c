#include <vsu/plt/backend/fb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <err.h>

/* */

static inline uint32_t rgb(vsuplt_fb_ptr fb, uint32_t r, uint32_t g, uint32_t b)
{
	/* er, should we care 'bout endianness? */
	/* fb.h suggests usage of color.offset together with '<<' */
	/* although it does not really imply... anything */
	return (r << fb->vinf.red.offset) |
	 		(g << fb->vinf.green.offset) |
		    (b << fb->vinf.blue.offset);
}

void
vsuplt_fb_flush(vsuplt_fb_ptr fb)
{
	vsuplt_bmp_ptr bmp = fb->bmp;
    if (fb->bmp == NULL)
        err(1, "vsuplt_fb_flush: bmp = NULL");
	if (fb->fb_fd == -1)
        err(1, "vsuplt_fb_flush: bad fb_fd");
    if (fb->vinf.bits_per_pixel != 32)
        err(1, "vsuplt_fb_flush: inconvenient value of bits_per_pixel");

    uint8_t *buf_base;
    uint32_t xoffset, yoffset, line_length;

    buf_base = fb->buf;
    xoffset = fb->vinf.xoffset;
    yoffset = fb->vinf.yoffset;
    line_length = fb->finf.line_length;

    for (long y = 0; y < fb->fb_h; ++y) {
        uint32_t *buf = (uint32_t*)(buf_base + y * line_length);

        for (long x = 0; x < fb->fb_w; ++x) {
            vsuplt_clr clr = vsuplt_bmp_get(bmp, x, fb->fb_h - y - 1);
            uint32_t px = rgb(fb, VSUPLT_R(clr), VSUPLT_G(clr), VSUPLT_B(clr));
            buf[x] = px;
        }
    }
    off_t o = lseek(fb->fb_fd, yoffset * line_length + xoffset, SEEK_SET);
    if (o == -1) err(1, "vsuplt_fb_flush: failed to lseek()");
    write(fb->fb_fd, buf_base, fb->fb_h * fb->finf.line_length);
}

void
vsuplt_fb_redraw_flush(vsuplt_fb_ptr fb)
{
	(*fb->redraw)(fb);
    vsuplt_fb_flush(fb);
}

void
vsuplt_fb_open(vsuplt_fb_ptr fb)
{
	fb->fb_fd = open(fb->fb_fname, O_WRONLY);
	if (fb->fb_fd == -1)
		err(1, "vsuplt_fb_show: failed to open %s", fb->fb_fname);
	if (fb->fb_fd == -1) err(1, "bad fb_fd");

	int ret;
	/* Get fixed and variable screen info. */
	/* Neither docs or header specifies return codes */
	/* so we'll just ignore returned values */
	ret = ioctl(fb->fb_fd, FBIOGET_FSCREENINFO, &fb->finf);
	ret = ioctl(fb->fb_fd, FBIOGET_VSCREENINFO, &fb->vinf);

	fb->vinf.grayscale = 0;
	fb->vinf.bits_per_pixel = 32;
	ret = ioctl(fb->fb_fd, FBIOPUT_VSCREENINFO, &fb->vinf);
	ret = ioctl(fb->fb_fd, FBIOGET_VSCREENINFO, &fb->vinf);

	fb->fb_w = fb->vinf.xres_virtual;
	fb->fb_h = fb->vinf.yres_virtual;

    /* note: 4 = bits_per_pixels/8 */
    fb->buf_size = fb->finf.line_length * fb->vinf.yres_virtual;
    fb->buf = malloc(fb->buf_size);
    if (fb->buf == NULL) err(1, "vsuplt_fb_open: failed to malloc() buffer");
}

void
vsuplt_fb_free(vsuplt_fb_ptr fb)
{
    if (fb->buf != NULL) {
        free(fb->buf);
        fb->buf = NULL;
    }
    if (fb->fb_fd != -1) {
        close(fb->fb_fd);
        fb->fb_fd = -1;
    }
}

void
vsuplt_fb_show2(vsuplt_fb_ptr fb)
{
	vsuplt_fb_open(fb);
	if (fb->post_init != NULL)
		(fb->post_init)(fb);
	if (fb->bmp == NULL) err(1, "vsuplt_fb_show2: no bmp");
	fb->redraw(fb);
	vsuplt_fb_flush(fb);
	vsuplt_term_input(&fb->in);
}
