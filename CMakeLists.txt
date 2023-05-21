#-------------------------------------------------------------------------------
# Copyright (c) 2023 OpenSynergy GmbH.
#
# This software may not be used in any way or distributed without
# permission. All rights reserved.
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)

project(uhmitest VERSION 0.0.1 DESCRIPTION "Library and test application for UHMI" LANGUAGES C)

add_subdirectory(libuhmigl)
add_subdirectory(es2gears)
add_subdirectory(uhmitest)
