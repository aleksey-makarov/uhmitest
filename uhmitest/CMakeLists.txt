#-------------------------------------------------------------------------------
# Copyright (c) 2022 OpenSynergy GmbH.
#
# This software may not be used in any way or distributed without
# permission. All rights reserved.
#-------------------------------------------------------------------------------

add_executable(uhmitest uhmitest.c)

add_dependencies(uhmitest uhmigl es2gears)
target_compile_options(uhmitest PRIVATE -Wall -Wextra -pedantic -Werror -O2)
target_link_libraries(uhmitest PRIVATE uhmigl es2gears)

include(GNUInstallDirs)

install(TARGETS uhmitest RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
