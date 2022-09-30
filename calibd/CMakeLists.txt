# ----------------------------------------------------------------------------
# @file    calibd/CMakeLists.txt
# @brief   input to the CMake build system for building calib daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( CALIBD_DIR ${PROJECT_BASE_DIR}/calibd )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/PI )
include_directories( ${PROJECT_BASE_DIR}/common )

link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(calibd 
        ${CALIBD_DIR}/calibd.cpp 
        ${CALIBD_DIR}/calib_interface.cpp 
        )

target_link_libraries(calibd
        physik_instrumente
        network
        common
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
