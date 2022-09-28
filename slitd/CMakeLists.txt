# ----------------------------------------------------------------------------
# @file    slitd/CMakeLists.txt
# @brief   input to the CMake build system for building slit daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( SLITD_DIR ${PROJECT_BASE_DIR}/slitd )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/PI )
include_directories( ${PROJECT_BASE_DIR}/common )

link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(slitd 
        ${SLITD_DIR}/slitd.cpp 
        ${SLITD_DIR}/slit_interface.cpp 
        )

target_link_libraries(slitd
        physik_instrumente
        network
        common
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
