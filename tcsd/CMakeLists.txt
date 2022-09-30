# ----------------------------------------------------------------------------
# @file    tcsd/CMakeLists.txt
# @brief   input to the CMake build system for building tcs daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( TCSD_DIR ${PROJECT_BASE_DIR}/tcsd )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(tcsd 
        ${TCSD_DIR}/tcsd.cpp 
        ${TCSD_DIR}/tcs_interface.cpp 
        )

target_link_libraries(tcsd
        network
        common
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
