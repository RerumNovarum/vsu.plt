#ifndef _PLT_SHOWPLOT_H_
#define _PLT_SHOWPLOT_H_

#include <vsu/plt.h>
#include <vsu/num.h>

struct vsuplt_camera2
{
    /* [0,W]\times [0,H] window contained in g is copied to device */
    size_t W, H; /* device-space window dimensions */
    RR L, R, B, T; /* the user-space window requested by interactive user */
    vsuplt_ctx *g; /* source of graphics content */
    void *device; /* device-specific data; must be ignored by client */
    void (*redraw)(struct vsuplt_camera2 *cam); /* callback to update the plot, set by client */
    int (*flush_to_device)(struct vsuplt_camera2 *cam); /* flush buffer from g to the device */
    void (*post_init)(struct vsuplt_camera2 *cam);
    int (*on_click_prim)(struct vsuplt_camera2 *cam); /* mouse click callback; if cb returns true, image will be redrawn */
};

void
vsuplt_camera2_show(struct vsuplt_camera2 *cam);
#endif
