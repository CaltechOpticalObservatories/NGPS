# ----------------------------------------------------------------------------
# @file    CMakeLists.txt
# @brief   top-level CMake input file for NGPS
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

add_definitions(-DLOGLEVEL_DEBUG)

#add_definitions( -Wall -ansi -O2 -Wno-variadic-macros -std=c++17 -ggdb )

cmake_minimum_required( VERSION 3.12 )

project( NGPS )

set( PROJECT_BASE_DIR $ENV{PWD}/../ )

set( PROJECT_SRC_DIR ${PROJECT_BASE_DIR}/src )
set( PROJECT_INC_DIR ${PROJECT_BASE_DIR}/include )
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_BASE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${PROJECT_BASE_DIR}/lib )
set( CMAKE_C_COMPILER /usr/bin/g++ )
set( CMAKE_CXX_COMPILER /usr/bin/g++ )

find_package( Threads )

#add_subdirectory( ${PROJECT_BASE_DIR}/AndorCamera2 )
add_subdirectory( ${PROJECT_BASE_DIR}/Andor )
add_subdirectory( ${PROJECT_BASE_DIR}/acamd )
add_subdirectory( ${PROJECT_BASE_DIR}/utils )
add_subdirectory( ${PROJECT_BASE_DIR}/common )
add_subdirectory( ${PROJECT_BASE_DIR}/camerad )
add_subdirectory( ${PROJECT_BASE_DIR}/sequencerd )
add_subdirectory( ${PROJECT_BASE_DIR}/flexured )
add_subdirectory( ${PROJECT_BASE_DIR}/calibd )
#add_subdirectory( ${PROJECT_BASE_DIR}/filterd )
add_subdirectory( ${PROJECT_BASE_DIR}/slitd )
add_subdirectory( ${PROJECT_BASE_DIR}/focusd )
add_subdirectory( ${PROJECT_BASE_DIR}/powerd )
add_subdirectory( ${PROJECT_BASE_DIR}/telemd )
add_subdirectory( ${PROJECT_BASE_DIR}/thermald )
add_subdirectory( ${PROJECT_BASE_DIR}/emulator )
add_subdirectory( ${PROJECT_BASE_DIR}/tcsd )
add_subdirectory( ${PROJECT_BASE_DIR}/PI )
add_subdirectory( ${PROJECT_BASE_DIR}/WTI )
add_subdirectory( ${PROJECT_BASE_DIR}/LKS )
add_subdirectory( ${PROJECT_BASE_DIR}/ARC )
