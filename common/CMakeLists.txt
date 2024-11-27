# ----------------------------------------------------------------------------
# @file    common/CMakeLists.txt
# @brief   input to CMake build system for building common software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)

set(PROJECT_UTILS_DIR ${PROJECT_BASE_DIR}/common)

set( CMAKE_CXX_STANDARD 17 )

include_directories( ${PROJECT_BASE_DIR}/common )
include_directories( ${PROJECT_BASE_DIR}/utils )    # needed for logentry

find_path( PYTHON_DEV "Python.h" PATHS /usr/include/python3.9 )
find_library( PYTHON_LIB python3.9 NAMES libpython3.9 PATHS /usr/lib64 )
find_library( ZMQPP_LIB zmqpp NAMES libzmqpp PATHS /usr/local/lib )
find_library( ZMQ_LIB zmq NAMES libzmq PATHS /usr/local/lib )

include_directories( ${PYTHON_DEV} )

add_library(common STATIC
        ${PROJECT_UTILS_DIR}/common.cpp
        ${ZMQPP_LIB}
        ${ZMQ_LIB}
        )
add_library(skyinfo STATIC
        ${PROJECT_UTILS_DIR}/skyinfo.cpp
        ${PYTHON_LIB}
        )
