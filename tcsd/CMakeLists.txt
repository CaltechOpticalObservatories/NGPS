# ----------------------------------------------------------------------------
# tcsd/CMakeLists.txt
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( TCSD_DIR ${PROJECT_BASE_DIR}/tcsd )

include_directories( ${PROJECT_BASE_DIR}/utils )
link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(tcsd 
        ${TCSD_DIR}/tcsd.cpp 
        ${TCSD_DIR}/tcs_interface.cpp 
        )

target_link_libraries(tcsd
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
