# ----------------------------------------------------------------------------
# @file    WTI/CMakeLists.txt
# @brief   input to the CMake build system for building WTI software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------
cmake_minimum_required( VERSION 3.12 )

set( WTI_DIR ${PROJECT_BASE_DIR}/WTI )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

add_library( wti STATIC
        ${WTI_DIR}/wti.cpp
        )
