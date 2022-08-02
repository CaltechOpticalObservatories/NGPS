# ----------------------------------------------------------------------------
# calibd/CMakeLists.txt
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
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
