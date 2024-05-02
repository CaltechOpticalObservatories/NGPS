# ----------------------------------------------------------------------------
# @file    Andor/CMakeLists.txt
# @brief   input to the CMake build system for building Andor software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

add_definitions( -Wall -ansi -O2 -Wno-variadic-macros -ggdb )

set( ANDOR_DIR ${PROJECT_BASE_DIR}/Andor )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )
include_directories( ${PYTHON_DEV} )

# Andor SDK
#
find_library( ANDOR_SDK andor NAMES libandor PATHS /usr/local/lib )

# CFITSIO, CCfits
#
find_library( CCFITS_LIB CCfits NAMES libCCfits PATHS /usr/local/lib )
find_library( CFITS_LIB cfitsio NAMES libcfitsio  PATHS /usr/local/lib )

# C-Python API Development libraries and headers
#
find_path( PYTHON_DEV "Python.h" PATHS /usr/include/python3.9 )
find_library( PYTHON_LIB python3.9 NAMES libpython3.9 PATHS /usr/lib64 )
add_link_options( -L/usr/lib64 -lpython3.9 -lcrypt -lpthread -ldl  -lutil -lm -lm )

add_library( andor STATIC
        ${ANDOR_DIR}/andor.cpp
        ${ANDOR_DIR}/sim_andor.cpp
        ${PYTHON_DEV}
        )

target_link_libraries( andor
        ${ANDOR_SDK}
        ${PYTHON_LIB}
        )
