#include <vsu/showplot.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    struct vsuplt_showplot_ctx plot = { 0 };
    plot.W = 1024;
    plot.H = 768;
    vsuplt_showplot(&plot);
    pause();
    return EXIT_SUCCESS;
}
