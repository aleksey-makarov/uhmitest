#-------------------------------------------------------------------------------
# Copyright (c) 2023 OpenSynergy GmbH.
#
# This software may not be used in any way or distributed without
# permission. All rights reserved.
#-------------------------------------------------------------------------------

add_library(es2gears OBJECT)

add_dependencies(es2gears uhmigl)

target_sources(es2gears PRIVATE es2gears.c)
target_link_libraries(es2gears uhmigl m)
target_include_directories (es2gears PRIVATE ${CMAKE_BINARY_DIR}/libuhmigl ../libuhmigl ../libuhmigl/include)
