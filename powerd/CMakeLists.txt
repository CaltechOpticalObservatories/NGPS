# ----------------------------------------------------------------------------
# powerd/CMakeLists.txt
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( POWERD_DIR ${PROJECT_BASE_DIR}/powerd )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )
include_directories( ${PROJECT_BASE_DIR}/WTI )
link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(powerd 
        ${POWERD_DIR}/powerd.cpp 
        ${POWERD_DIR}/power_interface.cpp 
        )

target_link_libraries(powerd
        wti
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
