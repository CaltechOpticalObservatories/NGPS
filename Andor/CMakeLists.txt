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

# Andor SDK
#
find_library( ANDOR_SDK andor NAMES libandor PATHS /usr/local/lib )

# CFITSIO, CCfits
#
find_library( CCFITS_LIB CCfits NAMES libCCfits PATHS /usr/local/lib )
find_library( CFITS_LIB cfitsio NAMES libcfitsio  PATHS /usr/local/lib )

add_library( andor STATIC
        ${ANDOR_DIR}/andor.cpp
        ${ANDOR_DIR}/sim.cpp
        )

target_link_libraries( andor
        ${ANDOR_SDK}
        )
