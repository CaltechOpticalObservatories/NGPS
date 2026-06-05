# ----------------------------------------------------------------------------
# @file    watchdog_system/CMakeLists.txt
# @brief   builds ngps-watchdog
# @details Mostly standalone (raw sockets + stderr logging, no daemon/common
#          libraries). Links ZeroMQ (zmqpp/zmq) only to probe the messaged
#          broker the same way clients do, plus pthread for sleep_for. Includes
#          common/ for sd_notify.h and common_commands.h.
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)

set( CMAKE_CXX_STANDARD 17 )

add_definitions( -Wall -Wextra -Wconversion -Wshadow -Og -Wno-variadic-macros -ggdb )

find_package( Threads REQUIRED )
find_library( ZMQPP_LIB zmqpp NAMES libzmqpp PATHS /usr/local/lib )
find_library( ZMQ_LIB   zmq   NAMES libzmq   PATHS /usr/local/lib )

add_executable( ngps-watchdog
        ${CMAKE_CURRENT_SOURCE_DIR}/ngps_watchdog.cpp
        )

target_include_directories( ngps-watchdog PRIVATE
        ${PROJECT_BASE_DIR}/common
        )

target_link_libraries( ngps-watchdog PRIVATE Threads::Threads ${ZMQPP_LIB} ${ZMQ_LIB} )
