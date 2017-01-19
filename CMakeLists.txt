cmake_minimum_required(VERSION 3.2)

project(vsuplt)

# import exported vsunum-config.cmake
# if target vsunum is not defined yet
# (i.e. import, unless building root project
#  containing vsunum as subdirectory)
if(NOT TARGET vsunum)
    find_package(vsunum)
endif()

add_library(vsuplt src/bresenham-line.c src/fill-poly.c
    src/plotgraph.c
    src/bmp.c src/plot2.c src/wireframe.c
    src/fb.c src/terminal-input.c)
target_link_libraries(vsuplt vsunum)

# REMINDER
# --------
# target_include_directories(target PUBLIC $d)
# includes headers in $d for both building and interface
# (i.e. these headers will be accessible transitively
#  to all dependent targets)
#
# here cmake requires some ugly tricks
# if $d was 'include'
#   it would expand as `pwd`/include
#   which would make install target fail
# so, generators $<BUILD_INTERFACE> $<INSTALL_INTERFACE>
# are used (`man 7 cmake-generator-expressions`);
#
# note that $<BUILD_INTERFACE:include> would also fail,
# probably it must expand to an absolute path

target_include_directories(vsuplt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

add_executable(wireframe src/show-wireframe.c)
target_link_libraries(wireframe vsuplt)

# REMINDER
# --------
# libvsuplt.a will be installed to ${PREFIX}/lib/
# (e.g. /usr/lib/libvsuplt.a)
install(TARGETS vsuplt
    EXPORT vsuplt-config
    DESTINATION lib)
# will install ${PREFIX}/include/vsu/plt.h
# NOTA BENE: slash at the end of 'include/' is mandatory
# otherwise installation path would contain excessive 'include' like below:
# ${CMAKE_INSTALL_PREFIX}/include/include/vsu/plt.h
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY assets DESTINATION share/libvsu/)
# ${PREFIX}/share/libvsu/plt/vsuplt-config.cmake
# for importing libvsuplt as installed target
install(EXPORT vsuplt-config DESTINATION share/libvsu/)

add_executable(test_fb_interactive tests/fb-interactive.c)
target_link_libraries(test_fb_interactive vsuplt)

add_executable(test_vsuplt_lines_interactive tests/test_vsuplt_lines_interactive.c)
target_link_libraries(test_vsuplt_lines_interactive vsuplt)

add_executable(cardioid src/show-plotgraph-cardioid.c)
target_link_libraries(cardioid vsuplt)

add_executable(cubspline src/showcubspline.c)
target_link_libraries(cubspline vsuplt)

enable_testing()

add_executable(test_plt_api tests/test_vsuplt_pub_api.c)
target_link_libraries(test_plt_api vsuplt)
add_test(test_plt_api test_plt_api)

add_executable(test_plot2_wireframe tests/wireframe.c)
target_link_libraries(test_plot2_wireframe vsuplt)
add_test(test_plot2_wireframe test_plot2_wireframe)

add_executable(test_fb_flush tests/fb-flush.c)
target_link_libraries(test_fb_flush vsuplt)
add_test(test_fb_flush test_fb_flush)

add_executable(test_bmp_triangle tests/fill-poly.c)
target_link_libraries(test_bmp_triangle vsuplt)
add_test(test_bmp_triangle test_bmp_triangle)
