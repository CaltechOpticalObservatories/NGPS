# ----------------------------------------------------------------------------
# @file    utils/CMakeLists.txt
# @brief   input to the CMake build system for building utils
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)

set(PROJECT_UTILS_DIR ${PROJECT_BASE_DIR}/utils)

add_library(utilities STATIC
        ${PROJECT_UTILS_DIR}/utilities.cpp
        )

add_library(logentry STATIC
        ${PROJECT_UTILS_DIR}/logentry.cpp
        )

add_library(network STATIC
        ${PROJECT_UTILS_DIR}/network.cpp
        )

# -- MySQL database ------------------------------------------------------------
#
set( MYSQL_DIR "/usr/local/mysql/connector" )
find_path( MYSQL_API "mysqlx/xdevapi.h" PATHS ${MYSQL_DIR}/include )
set( MYSQL_INCLUDES "${MYSQL_API}" )
find_library( MYSQL_LIB "mysqlcppconn8" NAMES "mysqlcppconn8" PATHS ${MYSQL_DIR}/lib64 )
include_directories( ${MYSQL_INCLUDES} )

add_library(database STATIC
        ${PROJECT_UTILS_DIR}/database.cpp
        ${MYSQL_LIB}
        )

add_executable(socksend
        ${PROJECT_UTILS_DIR}/sendcmd.c
        )

add_executable(listener
        ${PROJECT_UTILS_DIR}/listener.c
        )
