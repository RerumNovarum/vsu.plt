#include <vsu/showplot.h>

#include <xcb/xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
vsuplt_showplot(struct vsuplt_showplot_ctx *ctx)
{
    xcb_connection_t *c;
    xcb_screen_t *scr;
    xcb_drawable_t win;
    
    c = xcb_connect(NULL, NULL);
    scr = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
    win = xcb_generate_id(c);
    xcb_create_window(c,                    /* connection */
                      XCB_COPY_FROM_PARENT, /* depth */
                      win,                  /* window id */
                      scr->root,            /* parent window */
                      0, 0,                 /* initial position */
                      ctx->W, ctx->H,       /* window' width, height */
                      2,                    /* border width */
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, /* window class */
                      scr->root_visual,  /* visual */
                      0, NULL);             /* required by api */
    /* map window on the screen; until that it is invisible */
    xcb_map_window(c, win);
    xcb_flush(c);


}
