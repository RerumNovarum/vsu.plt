#include <vsu/camera2.h>

#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <cairo.h>

/* it turned out that /dev/fb0 can no longer be mmap()-ed */
/* with most modern graphics drivers in linux. */
/* it is not clear what other possible use of /dev/fb0 there can be. */
/* whatever, i have to use this stub now,
 * because i don't have time to learn something less horrible than gtk */

struct vsuplt_cairo_dev
{
    cairo_surface_t *cairo_buf;
    struct vsuplt_camera2 *cam;
    GtkWidget *canvas;
    gdouble lastx, lasty;
    gboolean navig_pressed;
};

static void destroy_cb(GtkApplication *app, struct vsuplt_cairo_dev *dev)
{
    if (dev == NULL) return;
    cairo_surface_destroy(dev->cairo_buf);
    if (dev->cam != NULL) {
        if (dev->cam->g != NULL) {
            vsuplt_destroy_ctx(dev->cam->g);
            free(dev->cam->g);
        }
    }
}

static gboolean
button_cb(GtkWidget *w, GdkEventButton *e, struct vsuplt_cairo_dev *dev)
{
    if (dev == NULL) return FALSE;
    if (dev->cam == NULL) return FALSE;
    if (e->state & GDK_BUTTON_PRIMARY) {
        if (dev->cam->click_prim == NULL) return FALSE;
        if ((*(dev->cam->click_prim))(dev->cam)) {
            GtkAllocation allocation;
            gtk_widget_get_allocation(w, &allocation);
            gtk_widget_queue_draw_area(w, 0, 0, allocation.width, allocation.height);
        }
    }
    return TRUE;
}

int vsuplt_camera2_navig_translate(struct vsuplt_camera2 *cam, RR dx, RR dy)
{
    cam->L += -dx;
    cam->R += -dx;
    cam->B += dy;
    cam->T += dy;
    return 1;
}

static gpointer
motion_cb(GtkWidget *w, GdkEventMotion *e, struct vsuplt_cairo_dev *dev)
{
    if (dev == NULL || dev->cam == NULL) return FALSE;

    if (e->state & GDK_BUTTON1_MASK) {
        if (dev->navig_pressed) {
            gdouble dx = e->x - dev->lastx;
            gdouble dy = e->y - dev->lasty;
            GtkAllocation allocation;
            gtk_widget_get_allocation(w, &allocation);
            dx /= allocation.width;
            dy /= allocation.height;
            if (dev->cam->nav_tr != NULL) {
                (*(dev->cam->nav_tr))(dev->cam, -dx, dy);
            }
            gtk_widget_queue_draw_area(w, 0, 0, allocation.width, allocation.height);
        }
        dev->navig_pressed = e->state & GDK_BUTTON1_MASK;
        dev->lastx = e->x;
        dev->lasty = e->y;
    } else {
        dev->navig_pressed = 0;
    }
}

static void vsuplt_cairo_setup_dev(struct vsuplt_cairo_dev *dev)
{
    GtkWidget *c = dev->canvas;
    GtkAllocation allocation;
    gtk_widget_get_allocation(c, &allocation);
    size_t cairo_w;
    size_t cairo_h;
    if (dev->cairo_buf != NULL) {
        cairo_w = cairo_image_surface_get_width(dev->cairo_buf);
        cairo_h = cairo_image_surface_get_height(dev->cairo_buf);   
    } else {
        cairo_w = 0;
        cairo_h = 0;
    }
    size_t win_w = allocation.width;
    size_t win_h = allocation.height;
    if (win_w == cairo_w && win_h == cairo_h)
        return;

    if (win_w == 0 || win_h == 0) {
        win_w = 128;
        win_h = 128;
    }
    if (dev->cairo_buf == NULL ||
        cairo_w < win_w ||
        cairo_h < win_h) {
        if (dev->cairo_buf != NULL) {
            cairo_surface_destroy(dev->cairo_buf);
        }
        cairo_w = 2*win_w;
        cairo_h = 2*win_h;
        cairo_surface_t *surface = cairo_image_surface_create(
                CAIRO_FORMAT_RGB24,
                cairo_w,
                cairo_h);
        dev->cairo_buf = surface;
    }

    struct vsuplt_camera2 *cam = dev->cam;
    struct vsuplt_ctx *g = cam->g;
    if (g == NULL || g->w < cairo_w || g->h < cairo_h)
    {
        if (g == NULL) {
            g = malloc(sizeof(struct vsuplt_ctx));
            cam->g = g;
        } else {
            vsuplt_destroy_ctx(g);
            *g = (struct vsuplt_ctx) { 0 };
        }
        vsuplt_init_ctx_alloc(g, cairo_w, cairo_h);
    }
    size_t mindim = win_w;
    if (win_h < win_w)
        mindim = win_h;
    /* also drops offset and rotation */
    g->ctm.T = affine2scale(mindim, mindim);
}

static gboolean draw_cb(GtkWidget *wid,
                    cairo_t *cr,
                    gpointer data)
{
    struct vsuplt_cairo_dev *dev = data;
    vsuplt_cairo_setup_dev(dev);
    struct vsuplt_camera2 *cam = dev->cam;
    struct vsuplt_ctx *g = cam->g;
    /* assuming RGB24 */
    size_t buf_w = cairo_image_surface_get_width(dev->cairo_buf);
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, buf_w);
    unsigned char *buf = (unsigned char *) cairo_image_surface_get_data(dev->cairo_buf);
    cam->redraw(cam);
    GtkAllocation allocation;
    gtk_widget_get_allocation(wid, &allocation);
    cairo_set_source_rgb(cr, 0xff, 0xff, 0xff);
    cairo_paint(cr);
    for (int y = 0; y < allocation.height; ++y) {
        uint32_t *row = (uint32_t*)(buf + y*stride);
        for (int x = 0; x < allocation.width; ++x) {
            PIXEL px = vsuplt_get_px(g, x, allocation.height - y - 1);
            row[x] = (VSUPLT_R(px) << 16) | (VSUPLT_G(px) << 8) | (VSUPLT_B(px));
        }
    }
    cairo_surface_flush(dev->cairo_buf);
    cairo_surface_mark_dirty(dev->cairo_buf);
    cairo_set_source_surface(cr, dev->cairo_buf, 0, 0);
    cairo_paint(cr);

    return FALSE;
}

static void
activate(GApplication *app,
         gpointer user_data)
{
    struct vsuplt_cairo_dev *dev = user_data;
    struct vsuplt_camera2 *cam = dev->cam;

    GtkWidget *w;
    GtkWidget *frame;
    GtkWidget *canvas;
    
    w = gtk_application_window_new(GTK_APPLICATION(app));
    frame = gtk_frame_new(NULL);
    canvas = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(w), frame);
    gtk_container_add(GTK_CONTAINER(frame), canvas);

    gtk_widget_set_size_request(canvas, 512, 512);

    g_signal_connect(w, "destroy", G_CALLBACK(destroy_cb), user_data);
    g_signal_connect(canvas, "draw", G_CALLBACK(draw_cb), user_data);
    g_signal_connect(canvas, "motion-notify-event", G_CALLBACK(motion_cb), user_data);
    g_signal_connect(canvas, "button-press-event", G_CALLBACK(button_cb), user_data);

    gtk_widget_set_events(canvas, gtk_widget_get_events(canvas)
                                  | GDK_POINTER_MOTION_MASK
                                  | GDK_BUTTON_PRESS_MASK);

    dev->canvas = canvas;
    gtk_widget_show_all(w);
}

void
vsuplt_camera2_show(struct vsuplt_camera2 *cam)
{
    GtkApplication *app;
    int status;

    struct vsuplt_cairo_dev dev = { 0 };
    dev.cam = cam;

    app = gtk_application_new("libvsu.plt.camera2", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &dev);
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
}
