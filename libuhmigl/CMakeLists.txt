add_library(uhmigl SHARED libuhmigl.c drm_state.c src/gles2.c src/egl.c)

target_compile_options(uhmigl PRIVATE -Wall -Wextra -pedantic -Werror -O2 ${extlibs_CFLAGS_OTHER})
target_include_directories(uhmigl PRIVATE ${extlibs_INCLUDE_DIRS} include)

set_target_properties(uhmigl PROPERTIES
    VERSION 0.1
    SOVERSION 1
    PUBLIC_HEADER libuhmigl.h)

include(GNUInstallDirs)

configure_file(libuhmigl.pc.in libuhmigl.pc @ONLY)

install(TARGETS uhmigl
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libuhmigl.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libuhmigl)
