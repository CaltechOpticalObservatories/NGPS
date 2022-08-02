# ----------------------------------------------------------------------------
# sequencer/CMakeLists.txt
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( SEQUENCER_DIR ${PROJECT_BASE_DIR}/sequencerd )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )
link_directories( ${PROJECT_BASE_DIR}/lib )

# -- MySQL database ------------------------------------------------------------
#
set( MYSQL_DIR "/usr/local/mysql/connector-c++-8.0.29" )
find_path( MYSQL_API "mysqlx/xdevapi.h" PATHS ${MYSQL_DIR}/include )
set( MYSQL_INCLUDES "${MYSQL_API}" )
find_library( MYSQL_LIB "mysqlcppconn8" NAMES "mysqlcppconn8" PATHS ${MYSQL_DIR}/lib64/debug )
include_directories( ${MYSQL_INCLUDES} )

add_executable(sequencerd
        ${SEQUENCER_DIR}/sequencerd.cpp 
        ${SEQUENCER_DIR}/sequencer_interface.cpp 
        ${SEQUENCER_DIR}/sequence.cpp 
        ${MYSQL_INCLUDES}
        )

target_link_libraries(sequencerd
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        ${CCFITS_LIB}
        ${CFITS_LIB}
        ${MYSQL_LIB}
        )
