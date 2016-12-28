cmake_minimum_required(VERSION 3.2)

project(vsuplt)

# import exported vsunum-config.cmake
# if target vsunum is not defined yet
# (i.e. import, unless building root project
#  containing vsunum as subdirectory)
if(NOT TARGET vsunum)
    find_package(vsunum)
endif()

add_library(vsuplt src/plt.c src/plt-affine.c)
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
# ${PREFIX}/share/libvsu/plt/vsuplt-config.cmake
# for importing libvsuplt as installed target
install(EXPORT vsuplt-config DESTINATION share/libvsu/)

add_library(vsuplt_camera2-fb src/camera2-fb.c)
target_link_libraries(vsuplt_camera2-fb vsunum vsuplt)

find_package(PkgConfig)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
add_library(vsuplt_camera2-gtk src/camera2-gtk.c)
target_include_directories(vsuplt_camera2-gtk PRIVATE ${GTK_INCLUDE_DIRS})
target_link_libraries(vsuplt_camera2-gtk vsunum vsuplt ${GTK_LIBRARIES})
install(TARGETS vsuplt_camera2-gtk
    EXPORT vsuplt-config
    DESTINATION lib)

enable_testing()
add_executable(test_plt_api tests/test_vsuplt_pub_api.c)
target_link_libraries(test_plt_api vsuplt)
add_test(test_plt_api test_plt_api)
add_executable(test_camera2-fb tests/camera2-fb.c)
target_link_libraries(test_camera2-fb vsuplt_camera2-fb)
# add_test(test_camera2 test_camera2)
add_executable(test_camera2-gtk tests/camera2-gtk.c)
target_link_libraries(test_camera2-gtk vsuplt_camera2-gtk)
