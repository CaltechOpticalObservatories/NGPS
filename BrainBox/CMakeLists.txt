# ----------------------------------------------------------------------------
# @file    BrainBox/CMakeLists.txt
# @brief   input to the CMake build system for building BrainBox software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------
cmake_minimum_required( VERSION 3.12 )

set( BRAINBOX_DIR ${PROJECT_BASE_DIR}/BrainBox )

set( CMAKE_CXX_STANDARD 17 )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

add_library( brainbox STATIC
        ${BRAINBOX_DIR}/brainbox.cpp
        )
