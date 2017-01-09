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
	if (fb->fb_fd == -1)
		 err(1, "vsuplt_fb_draw_bmp: bad fb_fd");
	if (fb->vinf.bits_per_pixel != 32)
		 err(1, "vsuplt_fb_draw_bmp: inconvenient value of bits_per_pixel");
	for (long y = 0; y < fb->fb_h; ++y) {
		off_t o = fb->vinf.xoffset + (y + fb->vinf.yoffset)*fb->finf.line_length;
		o = lseek(fb->fb_fd, o, SEEK_SET);
		if (o == -1) continue;
		for (long x = 0; x < fb->fb_w; ++x) {
			vsuplt_clr clr = vsuplt_bmp_get(bmp, x, y);
			uint32_t px = rgb(fb, VSUPLT_R(clr), VSUPLT_G(clr), VSUPLT_B(clr));
			if (write(fb->fb_fd, &px, 4) != 4) {
				err(1, "vsuplt_fb_draw_bmp failed to write");
			}
		}
	}
}

void _reload_dev_info(vsuplt_fb_ptr fb)
{
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
}
void _on_navig2(vsuplt_term_input_ptr incfg, char c)
{
	vsuplt_fb_ptr fb = incfg->self;
	struct affine2 T = AFFINE2_ID;
	switch (c)
	{
		case 'l': T.b1 = +fb->dx; break;
		case 'h': T.b1 = -fb->dx; break;
		case 'j': T.b2 = -fb->dy; break;
		case 'k': T.b2 = +fb->dy; break;
		case '+': T.a11 = fb->dscalex;     T.a22 = fb->dscaley; break;
		case '-': T.a11 = 1.0/fb->dscalex; T.a22 = 1.0/fb->dscaley; break;
		case 'r': T = affine2rot(fb->dphi); break;
		case 'R': T = affine2rot(-fb->dphi); break;
	}
	*(struct affine2*)fb->ctm = affine2mul(T, *(struct affine2*)fb->ctm);
	(*fb->redraw)(fb);
	vsuplt_fb_flush(fb);
}

void
vsuplt_fb_open(vsuplt_fb_ptr fb)
{
	fb->fb_fd = open(fb->fb_fname, O_WRONLY);
	if (fb->fb_fd == -1)
		err(1, "vsuplt_fb_show: failed to open %s", fb->fb_fname);
	_reload_dev_info(fb);
}

void
vsuplt_fb_show2(vsuplt_fb_ptr fb)
{
	vsuplt_fb_open(fb);
	if (fb->post_init != NULL)
		(fb->post_init)(fb);
	if (fb->bmp == NULL) err(1, "vsuplt_fb_show2: no bmp");
	for (int i = 0; i < sizeof(fb->in.handlers)/sizeof(*fb->in.handlers); ++i)
		fb->in.handlers[i] = NULL;
	fb->in.handlers['i'] = fb->in.handlers['j'] =
		fb->in.handlers['l'] = fb->in.handlers['h'] =
		fb->in.handlers['k'] = fb->in.handlers['+'] =
		fb->in.handlers['-'] = fb->in.handlers['r'] =
		fb->in.handlers['R'] = _on_navig2;
	fb->redraw(fb);
	vsuplt_fb_flush(fb);
	vsuplt_term_input(&fb->in);
}
