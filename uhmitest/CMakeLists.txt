#-------------------------------------------------------------------------------
# Copyright (c) 2022 OpenSynergy GmbH.
#
# This software may not be used in any way or distributed without
# permission. All rights reserved.
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.9)

project(uhmitest VERSION 0.0.1 DESCRIPTION "Test for libuhmigl" LANGUAGES C)

add_compile_options(-Wall -Wextra -pedantic -Werror -O2)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(pkglibs REQUIRED libuhmigl)

add_executable(uhmitest uhmitest.c es2gears.c)
target_link_libraries(uhmitest PRIVATE m ${pkglibs_LIBRARIES})
target_include_directories(uhmitest PRIVATE ${pkglibs_INCLUDE_DIRS})

install(TARGETS uhmitest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
