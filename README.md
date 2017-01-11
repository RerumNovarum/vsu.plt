libvsuplt
=========

Simplistic 2D graphics library written in plain C.

Structure
---------

```
vsu/
    plt/
        bmp.h /* bitmap */
        plot2.h /* plot with affine transforms &c */
        wireframe.h /* generic wireframe model structure */
        polymesh.h /* 3d polygonal meshmodel */
        backend/
                fb.h /* framebuffer output for bmp.h */
        util/
                alg.h /* reusable bresenham line drawing algo, filling shapes &c */
```

Examples
--------

See [show-wireframe.c](src/show-wireframe.c), [tests/*.c](tests/).

Documentation
------------

See [include/](include/)
