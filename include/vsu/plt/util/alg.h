#ifndef _VSUPLT_UTIL_ALG_H_
#define _VSUPLT_UTIL_ALG_H_

#include <vsu/num.h>

void
vsuplt_bresenhamline(void *self,
        void (*put_pixel)(void *s, int64_t x, int64_t y),
        int64_t x0, int64_t y0, int64_t x1, int64_t y1);

void
vsuplt_bresenhamline_window(void *self,
        void (*put_pixel)(void *s, int64_t x, int64_t y),
        int64_t L, int64_t R, int64_t B, int64_t T,
        int64_t x0, int64_t y0, int64_t x1, int64_t y1);

#endif /* ifndef _VSUPLT_UTIL_ALG_H_ */
