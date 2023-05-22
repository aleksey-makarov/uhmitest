#-------------------------------------------------------------------------------
# Copyright (c) 2023 OpenSynergy GmbH.
#
# This software may not be used in any way or distributed without
# permission. All rights reserved.
#-------------------------------------------------------------------------------

add_library(uhmigl SHARED)

option(GL_STATIC_LINKING "Don't use eglGetProcAddress() to load egl/gles functions" OFF)

include(FindPkgConfig)
pkg_check_modules(extlibs REQUIRED egl glesv2 libdrm gbm)

target_compile_options(uhmigl PRIVATE -Wall -Wextra -Werror -O2 ${extlibs_CFLAGS_OTHER})
target_include_directories(uhmigl PRIVATE ${extlibs_INCLUDE_DIRS} include ${CMAKE_BINARY_DIR}/libuhmigl)
target_link_libraries(uhmigl PUBLIC ${extlibs_LIBRARIES})
target_sources(uhmigl
    PRIVATE
        libuhmigl.c drm_state.c src/gles2.c src/egl.c loader.c pr.c egl_helpers.c
)

set_target_properties(uhmigl
    PROPERTIES
        VERSION 0.1
        SOVERSION 1
)

include(GNUInstallDirs)

configure_file(libuhmigl.pc.in libuhmigl.pc @ONLY)
configure_file(libuhmigl.h.in  libuhmigl.h  @ONLY)

install(TARGETS uhmigl
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libuhmigl.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libuhmigl.h pr.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libuhmigl
)

if(NOT GL_STATIC_LINKING)
install(DIRECTORY include
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libuhmigl
)
endif()
