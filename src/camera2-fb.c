#include <vsu/camera2.h>

#include <err.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h> /* mmap() */
#include <linux/fb.h> /* framebuffer */
#include <termios.h> /* to turn off canonical mode */

#define _DX 10
#define _DY 8

typedef int (*key_handler)(char c, struct vsuplt_camera2*);

struct vsuplt_console_dev
{
    struct fb_var_screeninfo *vinf; /* framebuffer info */
    struct fb_fix_screeninfo *finf; /* see linux/fb.h */
    struct termios saved_stdin_tattrs; /* saved terminal attributes */
    key_handler *key_handlers; /* ascii\to key_handler mapping */
    void* fb; /* framebuffer */
};

/* device-specific */

static inline uint32_t pixel_offset(
        uint32_t x, uint32_t y,
        uint32_t xoffset, uint32_t yoffset,
        uint32_t bpp, uint32_t line_len
        )
{
    return (x+xoffset)*(bpp >> 3) + (y+yoffset)*(line_len);
}

static inline uint32_t rgb(uint32_t r, uint32_t g, uint32_t b, struct fb_var_screeninfo *vinf)
{
    return (r << vinf->red.offset) | (g << vinf->green.offset) | (b << vinf->blue.offset);
}

static int flush_to_device(struct vsuplt_camera2 *cam)
{
    struct vsuplt_console_dev *dev = cam->device;
    uint32_t xoffset = dev->vinf->xoffset;
    uint32_t yoffset = dev->vinf->yoffset;
    uint32_t line_len = dev->finf->line_length;;
    uint32_t bpp = dev->vinf->bits_per_pixel;
    for (int y = 0; y < cam->g->h; ++y) {
        for (int x = 0; x < cam->g->w; ++x) {
            uint32_t offset = pixel_offset(x, y, xoffset, yoffset, bpp, line_len);
            PIXEL px = vsuplt_get_px(cam->g, x, y);
            uint32_t color = rgb(VSUPLT_R(px), VSUPLT_G(px), VSUPLT_B(px), dev->vinf);
            *(uint32_t*)(dev->fb + offset) = color;
        }
    }
    return 0;
}

/* input mode setup */

static inline void reset_stdin_mode(struct vsuplt_console_dev *dev)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &dev->saved_stdin_tattrs) < 0)
        err(1, "tcsetattr(): reset stdin");
}

static inline void set_stdin_mode(struct vsuplt_console_dev *dev)
{
    struct termios t;
    /* retrieve current terminal attribute s */
    if (tcgetattr(STDIN_FILENO, &t) < 0)
        err(1, "tcgetattr() failed;");
    dev->saved_stdin_tattrs = t;
    /* disable canonical mode and echoing */
    t.c_lflag &= ~(ECHO | ICANON);
    /* set min != 0 and time=0,
     * so that read() waits until there are at least min bytes in the queue
     */ 
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;

    /* commit changes */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t) < 0)
        err(1, "tcsetattr() failed;");
}

static int
ignore_key(char c, struct vsuplt_camera2 *cam) { return 0; }

static int
handle_exit_key(char c, struct vsuplt_camera2 *cam) {
    return 1;
}

static int
handle_navigation_key(char c, struct vsuplt_camera2 *cam)
{
    switch (c)
    {
        case 'h':
            cam->L -= _DX;
            cam->R -= _DX;
            break;
        case 'l':
            cam->L += _DX;
            cam->R += _DX;
            break;
        case 'j':
            cam->B -= _DY;
            cam->T -= _DY;
            break;
        case 'k':
            cam->B += _DY;
            cam->T += _DY;
            break;
    }
    cam->redraw(cam);
    if (cam->flush_to_device(cam) != 0)
        perror("flush_to_device()");
    return 0;
}

/* public routine */

void
vsuplt_camera2_show(struct vsuplt_camera2 *cam)
{
    int ret;
    int fb_fd;
    struct fb_var_screeninfo fbvarinf; /* framebuffer info */
    struct fb_fix_screeninfo fbfixinf; /* see linux/fb.h */

    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1) {
        err(1, "can not open /dev/fb0");
    }

    /* get fixed and variable screen info */
    /* neither docs or header specifies return codes */
    /* so let's pretend these ioctl's never fail */
    ret = ioctl(fb_fd, FBIOGET_FSCREENINFO, &fbfixinf);
    ret = ioctl(fb_fd, FBIOGET_VSCREENINFO, &fbvarinf);

    fbvarinf.grayscale = 0;
    fbvarinf.bits_per_pixel = 32;
    ret = ioctl(fb_fd, FBIOPUT_VSCREENINFO, &fbvarinf);
    ret = ioctl(fb_fd, FBIOGET_VSCREENINFO, &fbvarinf);
    
    cam->W = fbvarinf.xres_virtual;
    cam->H = fbvarinf.yres_virtual;

    vsuplt_ctx g;
    vsuplt_init_ctx_alloc(&g, cam->W, cam->H);
    cam->g = &g;
    vsuplt_scale(&g, cam->W, cam->H);

    size_t buflen = cam->W * fbfixinf.line_length;
    char *fb = mmap(NULL, buflen, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb == MAP_FAILED) {
        err(1, "can not mmap() framebuffer");
    }
    /* structure used for passing data to subroutines */
    struct vsuplt_console_dev condev = { .finf=&fbfixinf, .vinf=&fbvarinf, .fb=fb };
    cam->device = (void*)&condev;
    cam->flush_to_device = flush_to_device;

    char c;
    key_handler handlers[256];
    condev.key_handlers = handlers;
    for (int i = 0; i < 256; ++i) handlers[i] = ignore_key;
    handlers['h'] = handlers['i'] = handlers['j'] = handlers['k'] = handle_navigation_key;
    handlers[EOF] = handle_exit_key;

    if (cam->post_init != NULL) cam->post_init(cam);
    set_stdin_mode(&condev);
    while (read(STDIN_FILENO, &c, 1) > 0)
        if (handlers[c](c, cam) != 0) break;
    reset_stdin_mode(&condev);
}
