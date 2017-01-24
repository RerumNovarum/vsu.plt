#include <vsu/plt/util/alg.h>

/*
 * Bresenham line algorithm
 */

/* I: (x0, y0), (x1, y1) */
/* I: |y1-y0|<|x1-x0| */
/* I: x0 < x1 */
/* O: line segment from (x0, y0) to (x1, y1) */
/*    of color `clr` drawn on `bmp` */
void
inline static
_vsuplt_bresenhamline_x(void *self,
        void (*put_pixel)(void *s, int64_t x, int64_t y),
        int64_t x0, int64_t y0, int64_t x1, int64_t y1)
{
    if (x1 < x0)
    {
        int64_t tmp;
        tmp = x0; x0 = x1; x1 = tmp;
        tmp = y0; y0 = y1; y1 = tmp;
    }
    int64_t kx = x1 - x0;
    int64_t ky = y1 - y0;
    int dy = 1;
    if (ky < 0) dy = -1; /* sign(y1-y0) */
    ky = 2*ky*dy; /* 2|y1-y0| */
    kx = -2*kx; /* sign(y1-y0) * 2|x1-x0| */
    int64_t e = ky - (x1 - x0); /* 2H - W */
    put_pixel(self, x0, y0);
    for (int64_t x=x0, y=y0; x <= x1; ++x)
    {
        if (e > 0)
        {
            y += dy;
            e += kx; /* = e_{j-1} -/+ 2W */
        }
        e += ky; /* += 2H */
        put_pixel(self, x, y);
    }
}

void
inline static
_vsuplt_bresenhamline_y(void *self,
        void (*put_pixel)(void *s, int64_t x, int64_t y),
        int64_t x0, int64_t y0, int64_t x1, int64_t y1)
{

    if (y1 < y0)
    {
        int64_t tmp;
        tmp = x0; x0 = x1; x1 = tmp;
        tmp = y0; y0 = y1; y1 = tmp;
    }
    int64_t kx = x1 - x0;
    int64_t ky = y1 - y0;
    int dx = 1;
    if (kx < 0) dx = -1;
    kx = 2*kx*dx;
    ky = -2*ky;
    int64_t e = kx - (y1 - y0);
    put_pixel(self, x0, y0);
    for (int64_t y=y0, x=x0; y <= y1; ++y)
    {
        if (e > 0)
        {
            x += dx;
            e += ky;
        }
        e += kx;
        put_pixel(self, x, y);
    }
}

void
vsuplt_bresenhamline(void *self,
        void (*put_pixel)(void *s, int64_t x, int64_t y),
        int64_t x0, int64_t y0, int64_t x1, int64_t y1)
{
    register int32_t W = x1 - x0;
    if (W < 0) W = -W;
    register int32_t H = y1 - y0;
    if (H < 0) H = -H;
    if (W > H)
    {
        /* octants 1,4,5,8 */
        _vsuplt_bresenhamline_x(self, put_pixel, x0, y0, x1, y1);
    }
    else
    {
        /* 2,3,6,7 */
        _vsuplt_bresenhamline_y(self, put_pixel, x0, y0, x1, y1);
    }
}
