#include <vsu/num.h>
#include <vsu/plt/plot2.h>
#include <vsu/plt/wireframe.h>
#include <math.h>

#define IMG_W 1024
#define IMG_H 1024

int ret = 0;

char *wf_encoded = "42 43 2\n"
/* vertices */
/* 0 */  "176 530\n" "360 530\n" "390 450\n" "420 530\n"
/* 4 */  "540 530\n" "440 220\n" "350 220\n" "300 410\n"
/* 8 */  "350 410\n" "390 330\n" "340 330\n" "640 530\n"
/* 12 */ "645 410\n" "650 440\n" "660 410\n" "660 530\n"
/* 16 */ "760 530\n" "760 235\n" "735 220\n" "675 220\n"
/* 20 */ "650 240\n" "650 270\n" "640 220\n" "570 220\n"
/* 24 */ "560 230\n" "560 220\n" "500 220\n" "501 406\n"
/* 28 */ "890 530\n" "890 430\n" "895 445\n" "900 420\n"
/* 32 */ "900 520\n" "990 520\n" "990 235\n" "965 220\n"
/* 36 */ "890 220\n" "865 230\n" "855 220\n" "790 220\n"
/* 40 */ "770 230\n" "770 220\n"
/* edges */
/* 0 */  "0 1 0xFF0000\n"   "1 2 0xFF0000\n"    "2 3 0xFF0000\n"   "3 4 0xFF0000\n"
/* 4 */  "4 5 0xFF0000\n"   "5 6 0xFF0000\n"    "6 0 0xFF0000\n"   "7 8 0xFF0000\n"
/* 8 */  "8 9 0xFF0000\n"   "9 10 0xFF0000\n"   "11 12 0xFF0000\n" "13 14 0xFF0000\n"
/* 12 */ "15 16 0xFF0000\n" "16 17 0xFF0000\n"  "17 18 0xFF0000\n" "18 19 0xFF0000\n"
/* 16 */ "19 20 0xFF0000\n" "20 21 0xFF0000\n"  "20 22 0xFF0000\n" "22 23 0xFF0000\n"
/* 20 */ "23 24 0xFF0000\n" "24 25 0xFF0000\n"  "25 26 0xFF0000\n" "26 27 0xFF0000\n"
/* 24 */ "16 28 0xFF0000\n" "28 29 0xFF0000\n"  "29 30 0xFF0000\n" "30 31 0xFF0000\n"
/* 28 */ "31 32 0xFF0000\n" "33 34 0xFF0000\n"  "34 35 0xFF0000\n" "36 37 0xFF0000\n"
/* 32 */ "38 39 0xFF0000\n" "39 40 0xFF0000\n"  "40 41 0xFF0000\n" "41 18 0xFF0000\n"
/* breaking the order: */
"10 7 0xFF0000\n" "4 11 0xFF0000\n" "12 13 0xFF0000\n" "14 15 0xFF0000\n"
"32 33 0xFF0000\n" "35 36 0xFF0000\n" "37 38 0xFF0000\n";

int main()
{
    vsuplt_plot2_ptr plot = vsuplt_plot2_alloc(IMG_W, IMG_H,
            175, 994, 531, 219);
    FILE *in = fmemopen(wf_encoded, strlen(wf_encoded), "r");
    vsuplt_wireframe_ptr wf = vsuplt_wireframe_load(in);
    vsuplt_plot2_clear(plot, VSUPLT_COLOR_WHITE);
    //vsuplt_wireframe2_transform(wf, affine2tr(-175,-219));
    //vsuplt_wireframe2_transform(wf, affine2scale(IMG_W/(995-175), IMG_H/(530-219)));
    //vsuplt_wireframe2_transform(wf, affine2tr(+IMG_W/2,+IMG_H/2));
    //vsuplt_wireframe2_transform(wf, affine2rot(M_PI/6));
    //vsuplt_wireframe2_transform(wf, affine2tr(-IMG_W/2,-IMG_H/2));
    vsuplt_plot2_draw_wireframe(plot, wf);
    vsuplt_plot2_print_ppm_file(plot, "wireframe2.ppm");
    return ret;
}
