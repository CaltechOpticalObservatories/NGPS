# ----------------------------------------------------------------------------
# @file    common/CMakeLists.txt
# @brief   input to CMake build system for building common software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)

set(PROJECT_UTILS_DIR ${PROJECT_BASE_DIR}/common)

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

include_directories( ${PROJECT_BASE_DIR}/common )
include_directories( ${PROJECT_BASE_DIR}/utils )    # needed for logentry

find_package( Python3 COMPONENTS Development )
if ( Python3_FOUND )
  list( GET Python3_INCLUDE_DIRS 0 PYTHON_DEV )
  list( GET Python3_LIBRARIES 0 PYTHON_LIB )
else()
  find_path( PYTHON_DEV "Python.h" PATHS /usr/include/python3.9 )
  find_library( PYTHON_LIB NAMES python3.9 libpython3.9 PATHS /usr/lib64 )
endif()
find_library( ZMQPP_LIB NAMES zmqpp libzmqpp
              PATHS ${NGPS_ZMQ_SEARCH_PATHS} ${NGPS_ZMQ_FIND_MODE} )
find_library( ZMQ_LIB NAMES zmq libzmq
              PATHS ${NGPS_ZMQ_SEARCH_PATHS} ${NGPS_ZMQ_FIND_MODE} )

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
