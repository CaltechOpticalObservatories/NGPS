# ----------------------------------------------------------------------------
# @file    focusd/CMakeLists.txt
# @brief   input to the CMake build system for building focus daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( FOCUSD_DIR ${PROJECT_BASE_DIR}/focusd )

set( CMAKE_CXX_STANDARD 17 )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/PI )
include_directories( ${PROJECT_BASE_DIR}/common )

find_library( ZMQPP_LIB zmqpp NAMES libzmqpp PATHS /usr/local/lib )
find_library( ZMQ_LIB zmq NAMES libzmq PATHS /usr/local/lib )

link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(focusd 
        ${FOCUSD_DIR}/focusd.cpp 
        ${FOCUSD_DIR}/focus_server.cpp 
        ${FOCUSD_DIR}/focus_interface.cpp 
        )

target_link_libraries(focusd
        physik_instrumente
        network
        common
        logentry
        utilities
        ${ZMQPP_LIB}
        ${ZMQ_LIB}
        ${CMAKE_THREAD_LIBS_INIT}
        )

# -- Port Definitions ----------------------------------------------------------
#
# portdefs is the master list of ports for NGPS and is used to generate the
# individual .cfg files to avoid conflicts across multiple files, by keeping
# them all in one place..
#
INCLUDE( ${PROJECT_BASE_DIR}/Config/portdefs )
INCLUDE( ${PROJECT_BASE_DIR}/Config/privatelan )
INCLUDE( ${PROJECT_BASE_DIR}/Config/ets8p )
set ( FOCUSD_CFG_COMMENT "This file is automatically generated. Manual edits may be overwritten." )
CONFIGURE_FILE( ${PROJECT_BASE_DIR}/Config/focusd.cfg.in ${PROJECT_BASE_DIR}/Config/focusd.cfg )
