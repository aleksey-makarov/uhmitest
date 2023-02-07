#-------------------------------------------------------------------------------
# Copyright (c) 2023 OpenSynergy GmbH.
#
# This software may not be used in any way or distributed without
# permission. All rights reserved.
#-------------------------------------------------------------------------------

add_library(uhmigl SHARED)

include(FindPkgConfig)
pkg_check_modules(extlibs REQUIRED egl glesv2 libdrm gbm)

target_compile_options(uhmigl PRIVATE -Wall -Wextra -pedantic -Werror -O2 ${extlibs_CFLAGS_OTHER})
target_include_directories(uhmigl PRIVATE ${extlibs_INCLUDE_DIRS} include)
target_link_libraries(uhmigl PUBLIC ${extlibs_LIBRARIES})
target_sources(uhmigl
    PRIVATE
        libuhmigl.c drm_state.c src/gles2.c src/egl.c loader.c pr.c egl_helpers.c
    PUBLIC
        FILE_SET uhmigl_headers
            TYPE HEADERS
            FILES libuhmigl.h pr.h
        FILE_SET uhmigl_headers_glad
            TYPE HEADERS
            FILES include/EGL/eglplatform.h include/KHR/khrplatform.h include/glad/egl.h include/glad/gles2.h
            BASE_DIRS include
)

set_target_properties(uhmigl
    PROPERTIES
        VERSION 0.1
        SOVERSION 1
)

include(GNUInstallDirs)

install(TARGETS uhmigl
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET uhmigl_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libuhmigl
    FILE_SET uhmigl_headers_glad DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libuhmigl
)

configure_file(libuhmigl.pc.in libuhmigl.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libuhmigl.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
