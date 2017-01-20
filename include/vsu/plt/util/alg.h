#ifndef _VSUPLT_UTIL_ALG_H_
#define _VSUPLT_UTIL_ALG_H_

#include <vsu/num.h>
#include <stdbool.h>

void
vsuplt_bresenhamline(void *self,
        void (*put_pixel)(void *s, int64_t x, int64_t y),
        int64_t x0, int64_t y0, int64_t x1, int64_t y1);

void vsuplt_fill_triangle(void *self,
        void (*put_pixel)(void *s, int64_t x, int64_t y),
        int64_t X0, int64_t Y0,
        int64_t X1, int64_t Y1,
        int64_t X2, int64_t Y2);

#endif /* ifndef _VSUPLT_UTIL_ALG_H_ */
