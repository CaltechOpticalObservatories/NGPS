# ----------------------------------------------------------------------------
# @file    flexure/CMakeLists.txt
# @brief   input to the CMake build system for building flexure daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( FLEXURED_DIR ${PROJECT_BASE_DIR}/flexured )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(flexured 
        ${FLEXURED_DIR}/flexured.cpp 
        ${FLEXURED_DIR}/flexure_interface.cpp 
        )

target_link_libraries(flexured
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
