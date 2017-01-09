#ifndef _VSUPLT_FB_H_
#define _VSUPLT_FB_H_

#include <vsu/plt/util/terminal-input.h>
#include <vsu/plt/bmp.h>
#include <vsu/num.h>
#include <stdio.h>
#include <linux/fb.h> /* framebuffer */

struct vsuplt_fb;
typedef struct vsuplt_fb * vsuplt_fb_ptr;

struct vsuplt_fb
{
	struct vsuplt_term_input in;
	struct fb_fix_screeninfo finf;
	struct fb_var_screeninfo vinf;
	vsuplt_bmp_ptr bmp;
	int fb_fd;
	size_t fb_w, fb_h;
	char *fb_fname;
	void *userdata;
	void (*post_init)(vsuplt_fb_ptr);
	void (*redraw)(vsuplt_fb_ptr);
};

void
vsuplt_fb_show2(vsuplt_fb_ptr fb);

void
vsuplt_fb_flush(vsuplt_fb_ptr fb);

void
vsuplt_fb_redraw_flush(vsuplt_fb_ptr fb);

void
vsuplt_fb_open(vsuplt_fb_ptr fb);

#endif /* ifndef _VSUPLT_FB_H_ */
