# ----------------------------------------------------------------------------
# @file    Galil/CMakeLists.txt
# @brief   input to the CMake build system for building Galil software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------
cmake_minimum_required( VERSION 3.12 )

set( GALIL_DIR ${PROJECT_BASE_DIR}/Galil )

set( CMAKE_CXX_STANDARD 17 )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

add_library( galil STATIC
        ${GALIL_DIR}/galil.cpp
        )
