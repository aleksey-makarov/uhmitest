#-------------------------------------------------------------------------------
# Copyright (c) 2023 OpenSynergy GmbH.
#
# This software may not be used in any way or distributed without
# permission. All rights reserved.
#-------------------------------------------------------------------------------

add_library(es2gears OBJECT)

add_dependencies(es2gears uhmigl)

target_sources(es2gears
    PRIVATE
        es2gears.c
    PUBLIC
        FILE_SET es2gears_headers
            TYPE HEADERS
            FILES es2gears.h
)

target_compile_options(es2gears PRIVATE -Wall -Wextra -pedantic -Werror -O2)
target_link_libraries(es2gears uhmigl m)
