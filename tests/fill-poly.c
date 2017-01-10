#include <vsu/plt/bmp.h>
#include <vsu/plt/util/alg.h>

int main()
{
    vsuplt_bmp_ptr bmp = vsuplt_bmp_alloc(384, 384);
    vsuplt_bmp_triangle(bmp, 10, 10, 40, 260, 360, 20, VSUPLT_COLOR_RED);
    vsuplt_bmp_print_ppm_file(bmp, "triangle.ppm");
    vsuplt_bmp_free(bmp);
    return 0;
}
