#include <vsu/plt/util/alg.h>

static inline void _swap64(int64_t *a, int64_t *b)
{
    int64_t t = *a;
    *a = *b;
    *b = t;
}

static inline void _swap_two_x64_pairs(
        int64_t *a, int64_t *b,
        int64_t *c, int64_t *d)
{
    int64_t t = *a;
    *a = *b;
    *b = t;
    t = *c;
    *c = *d;
    *d = t;
}

static inline void order_3_points(
        int64_t *x0, int64_t *y0,
        int64_t *x1, int64_t *y1,
        int64_t *x2, int64_t *y2) {
    if (*x1 < *x0) _swap_two_x64_pairs(x0, x1, y0, y1);
    if (*x2 < *x0) _swap_two_x64_pairs(x0, x2, y0, y1);
    if (*x2 < *x1) _swap_two_x64_pairs(x1, x2, y0, y1);
    if (*x0 == *x1 && *y1 < *y0) _swap64(y0, y1);
    if (*x0 == *x2 && *y2 < *y0) _swap64(y0, y2);
    if (*x1 == *x2 && *y2 < *y1) _swap64(y1, y2);
}

struct _ft_data
{
    int64_t x0;
    int64_t *y;
};

void _ft_put_pixel(void *self, int64_t x, int64_t y)
{
    struct _ft_data *d = self;
    d->y[x - d->x0] = y;
}

void vsuplt_fill_triangle(void *self,
        void (*put_pixel)(void *s, int64_t x, int64_t y),
        int64_t X0, int64_t Y0,
        int64_t X1, int64_t Y1,
        int64_t X2, int64_t Y2)
{
    order_3_points(&X0, &Y0, &X1, &Y1, &X2, &Y2);
    int64_t *h1, *h2;
    size_t bytes_needed = 2*sizeof(int64_t)*(X2 - X0 + 1);
    h1 = malloc(bytes_needed);
    h2 = h1 + (X2 - X0 + 1);
    struct _ft_data d1 = { .x0=X0, .y=h1 },
                    d2 = { .x0=X0, .y=h2 };
    vsuplt_bresenhamline(&d1, _ft_put_pixel, X0, Y0, X1, Y1);
    vsuplt_bresenhamline(&d1, _ft_put_pixel, X1, Y1, X2, Y2);
    vsuplt_bresenhamline(&d2, _ft_put_pixel, X0, Y0, X2, Y2);

    for (int64_t x = X0; x <= X2; ++x) {
        int64_t y1 = h1[x - X0];
        int64_t y2 = h2[x - X0];
        if (y1 > y2) _swap64(&y1, &y2);
        for (int64_t y = y1; y <= y2; ++y) {
            put_pixel(self, x, y);
        }
    }
    free(h1);
}

// void
// vsuplt_fill_poly(void *self,
//         void (*put_pixel)(void *s, int64_t x, int64_t y),
//         bool (*is_hlted)(void *s, int64_t x, int64_t y),
//         int64_t (*get_x)(void *s, uint32_t v),
//         int64_t (*get_y)(void *s, uint32_t v),
//         size_t vertices_no)
// {
//     if (vertices_no == 0) return;
//     int64_t x, y, xp, yp;
//     xp = get_x(0);
//     yp = get_y(0);
//     for (int v = 1; v < vertices_no; ++v) {
//         x = get_x(v);
//         y = get_y(v);
//         vsuplt_bresenhamline(s, put_pixel, xp, yp, x, y);
//         xp = x;
//         yp = y;
//     }
//     x = get_x(0);
//     y = get_y(0);
//     vsuplt_bresenhamline(s, put_pixel, xp, yp, x, y);
// }
