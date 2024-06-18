# ----------------------------------------------------------------------------
# @file    LKS/CMakeLists.txt
# @brief   input to the CMake build system for building LKS software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------
cmake_minimum_required( VERSION 3.12 )

set( LKS_DIR ${PROJECT_BASE_DIR}/LKS )

set( CMAKE_CXX_STANDARD 17 )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

add_library( lks STATIC
        ${LKS_DIR}/lks.cpp
        )
