# ----------------------------------------------------------------------------
# @file    Andor/CMakeLists.txt
# @brief   input to the CMake build system for building Andor software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( ANDOR_DIR ${PROJECT_BASE_DIR}/Andor )

set( CMAKE_CXX_STANDARD 17 )

add_definitions( -Wall -O0 -Wno-variadic-macros -ggdb )
add_definitions(-D_LP64)

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

# Andor SDK
#
if ( NGPS_WITH_ANDOR_SDK )
  find_library( ANDOR_SDK andor NAMES libandor PATHS /usr/local/lib /opt/homebrew/lib )
endif()

# CFITSIO, CCfits
#
find_library( CCFITS_LIB CCfits NAMES libCCfits PATHS /usr/local/lib /opt/homebrew/lib )
find_library( CFITS_LIB cfitsio NAMES libcfitsio  PATHS /usr/local/lib /opt/homebrew/lib )

# C-Python API Development libraries and headers
#
find_package( Python3 COMPONENTS Development )
if ( Python3_FOUND )
  list( GET Python3_INCLUDE_DIRS 0 PYTHON_DEV )
  list( GET Python3_LIBRARIES 0 PYTHON_LIB )
else()
  find_path( PYTHON_DEV "Python.h" PATHS /usr/include/python3.9 )
  find_library( PYTHON_LIB NAMES python3.9 libpython3.9 PATHS /usr/lib64 )
endif()
include_directories( ${PYTHON_DEV} )
if ( UNIX AND NOT APPLE )
  add_link_options( -lcrypt -lpthread -ldl -lutil -lm -lm )
endif()

set( ANDOR_SOURCES
        ${ANDOR_DIR}/andor.cpp
        ${ANDOR_DIR}/andor_emulator.cpp
        )
if ( NOT NGPS_WITH_ANDOR_SDK )
  list( APPEND ANDOR_SOURCES ${PROJECT_BASE_DIR}/stubs/andor/atmcdLXd_stub.cpp )
endif()

add_library( andor STATIC
        ${ANDOR_SOURCES}
        ${PYTHON_DEV}
        )

target_link_libraries( andor
        ${PYTHON_LIB}
        )
if ( NGPS_WITH_ANDOR_SDK )
  target_link_libraries( andor ${ANDOR_SDK} )
endif()
