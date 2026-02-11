# ----------------------------------------------------------------------------
# @file    utils/CMakeLists.txt
# @brief   input to the CMake build system for building utils
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)

set(PROJECT_UTILS_DIR ${PROJECT_BASE_DIR}/utils)

set( CMAKE_CXX_STANDARD 17 )

add_definitions( -Wall -Wextra -Wconversion -Wshadow -Og -Wno-variadic-macros -ggdb )

include_directories( ${PROJECT_BASE_DIR}/utils )

set( STDCXXFS_LIB "" )
if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  set( STDCXXFS_LIB stdc++fs )
endif()

add_library(utilities STATIC
        ${PROJECT_UTILS_DIR}/utilities.cpp
        md5
        )
target_link_libraries(utilities PRIVATE ${STDCXXFS_LIB})

add_library(logentry STATIC
        ${PROJECT_UTILS_DIR}/logentry.cpp
        )
target_link_libraries( logentry ${STDCXXFS_LIB} )

add_library(network STATIC
        ${PROJECT_UTILS_DIR}/network.cpp
        )

add_library( md5 STATIC
        ${PROJECT_UTILS_DIR}/md5.c
        )

add_executable(make_directories
        ${PROJECT_UTILS_DIR}/make_directories.cpp
        )
target_link_libraries(make_directories PRIVATE
        ${STDCXXFS_LIB}
        utilities
        )

# -- MySQL database ------------------------------------------------------------
#
set( MYSQL_DIR "/usr/local/mysql/connector" )
find_path( MYSQL_API "mysqlx/xdevapi.h"
           PATHS ${MYSQL_DIR} /usr/local/opt/mysql-connector-c++ /opt/homebrew/opt/mysql-connector-c++
           PATH_SUFFIXES include )
set( MYSQL_INCLUDES "${MYSQL_API}" )
find_library( MYSQL_LIB NAMES mysqlcppconnx mysqlcppconn8 mysqlcppconn
              PATHS ${MYSQL_DIR} /usr/local/opt/mysql-connector-c++ /opt/homebrew/opt/mysql-connector-c++
              PATH_SUFFIXES lib lib64 )
include_directories( ${MYSQL_INCLUDES} )

add_library(database STATIC
        ${PROJECT_UTILS_DIR}/database.cpp
        )
if ( MYSQL_LIB )
  target_link_libraries( database PUBLIC ${MYSQL_LIB} )
endif()

add_executable(socksend
        ${PROJECT_UTILS_DIR}/sendcmd.c
        )

add_executable(listener
        ${PROJECT_UTILS_DIR}/listener.cpp
        )
target_link_libraries( listener PRIVATE utilities )

# -- ZeroMQ --------------------------------------------------------------------
#
find_library( ZMQPP_LIB NAMES zmqpp libzmqpp
              PATHS ${NGPS_ZMQ_SEARCH_PATHS} ${NGPS_ZMQ_FIND_MODE} )
find_library( ZMQ_LIB NAMES zmq libzmq
              PATHS ${NGPS_ZMQ_SEARCH_PATHS} ${NGPS_ZMQ_FIND_MODE} )

# -- X11 GUI tools ------------------------------------------------------------
#
find_package(X11)
if (X11_FOUND)
  add_executable(seq_progress_gui
          ${PROJECT_UTILS_DIR}/seq_progress_gui.cpp
          )
  target_include_directories(seq_progress_gui PRIVATE
          ${X11_INCLUDE_DIR}
          ${PROJECT_BASE_DIR}/common
          )
  target_link_libraries(seq_progress_gui PRIVATE
          ${STDCXXFS_LIB}
          utilities
          logentry
          network
          ${ZMQPP_LIB}
          ${ZMQ_LIB}
          ${X11_LIBRARIES}
          )
endif()
