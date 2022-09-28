# ----------------------------------------------------------------------------
# @file    PI/CMakeLists.txt
# @brief   input to the CMake build system for building PI software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( PI_DIR ${PROJECT_BASE_DIR}/PI )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

add_library( physik_instrumente STATIC
        ${PI_DIR}/pi.cpp
        )
