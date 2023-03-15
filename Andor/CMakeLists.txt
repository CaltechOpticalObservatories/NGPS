# ----------------------------------------------------------------------------
# @file    Andor/CMakeLists.txt
# @brief   input to the CMake build system for building Andor software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( ANDOR_DIR ${PROJECT_BASE_DIR}/Andor )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

find_library( ANDOR_SDK andor NAMES libandor PATHS /usr/local/lib )

add_library( andor STATIC
        ${ANDOR_DIR}/andor.cpp
        )

target_link_libraries( andor
        ${ANDOR_SDK}
        )
