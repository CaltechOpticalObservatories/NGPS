# ----------------------------------------------------------------------------
# @file    common/CMakeLists.txt
# @brief   input to CMake build system for building common software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)

set(PROJECT_UTILS_DIR ${PROJECT_BASE_DIR}/common)

include_directories( ${PROJECT_BASE_DIR}/common )
include_directories( ${PROJECT_BASE_DIR}/utils )    # needed for logentry

find_path( PYTHON_DEV "Python.h" PATHS /usr/include/python3.9 )
find_library( PYTHON_LIB python3.9 NAMES libpython3.9 PATHS /usr/lib64 )
include_directories( ${PYTHON_DEV} )

add_library(common STATIC
        ${PROJECT_UTILS_DIR}/common.cpp
        )
add_library(skyinfo STATIC
        ${PROJECT_UTILS_DIR}/skyinfo.cpp
        ${PYTHON_LIB}
        )
