# ----------------------------------------------------------------------------
# @file    focusd/CMakeLists.txt
# @brief   input to the CMake build system for building focus daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( FOCUSD_DIR ${PROJECT_BASE_DIR}/focusd )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(focusd 
        ${FOCUSD_DIR}/focusd.cpp 
        ${FOCUSD_DIR}/focus_interface.cpp 
        )

target_link_libraries(focusd
        network
        common
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
