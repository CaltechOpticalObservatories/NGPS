# ----------------------------------------------------------------------------
# CMakeLists.txt
# top-level CMake input file for NGPS
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

project( NGPS )

set( PROJECT_BASE_DIR $ENV{HOME}/Software/ )

set( PROJECT_SRC_DIR ${PROJECT_BASE_DIR}/src )
set( PROJECT_INC_DIR ${PROJECT_BASE_DIR}/include )
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_BASE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${PROJECT_BASE_DIR}/lib )
set( CMAKE_C_COMPILER /usr/bin/g++ )
set( CMAKE_CXX_COMPILER /usr/bin/g++ )

find_package( Threads )

add_subdirectory( ${PROJECT_BASE_DIR}/utils )
add_subdirectory( ${PROJECT_BASE_DIR}/camerad )
