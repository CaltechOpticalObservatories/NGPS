# ----------------------------------------------------------------------------
# @file    emulator/CMakeLists.txt
# @brief   input to CMake build system for building emulator software packages
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( EMULATOR_DIR ${PROJECT_BASE_DIR}/emulator )

include_directories( ${EMULATOR_DIR} )
include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )

link_directories( ${PROJECT_BASE_DIR}/lib )

find_path( PYTHON_DEV "Python.h" PATHS /usr/include/python3.9 )
find_library( PYTHON_LIB python3.9 NAMES libpython3.9 PATHS /usr/lib64 )
include_directories( ${PYTHON_DEV} )

add_definitions( -Wall -ansi -O2 -Wno-variadic-macros -std=c++17 -ggdb )

# ----------------------------------------------------------------------------
# andorserver
# ----------------------------------------------------------------------------

add_executable(emulatord.andorserver
        ${EMULATOR_DIR}/emulatord_andorserver.cpp 
        ${EMULATOR_DIR}/andorserver.cpp 
        )
target_link_libraries(emulatord.andorserver
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# acam_pi
# ----------------------------------------------------------------------------

add_executable(emulatord.acam_pi
        ${EMULATOR_DIR}/emulatord_acam_pi.cpp 
        ${EMULATOR_DIR}/acam_pi.cpp 
        )
target_link_libraries(emulatord.acam_pi
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# calib
# ----------------------------------------------------------------------------

add_executable(emulatord.calib
        ${EMULATOR_DIR}/emulatord_calib.cpp 
        ${EMULATOR_DIR}/calib.cpp 
        )
target_link_libraries(emulatord.calib
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# camera
# ----------------------------------------------------------------------------

add_executable(emulatord.camera
        ${EMULATOR_DIR}/emulatord_camera.cpp 
        ${EMULATOR_DIR}/camera.cpp 
        )
target_link_libraries(emulatord.camera
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# flexure
# ----------------------------------------------------------------------------

add_executable(emulatord.flexure
        ${EMULATOR_DIR}/emulatord_flexure.cpp 
        ${EMULATOR_DIR}/flexure.cpp 
        )
target_link_libraries(emulatord.flexure
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# focus
# ----------------------------------------------------------------------------

add_executable(emulatord.focus
        ${EMULATOR_DIR}/emulatord_focus.cpp 
        ${EMULATOR_DIR}/focus.cpp 
        )
target_link_libraries(emulatord.focus
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# power
# ----------------------------------------------------------------------------

add_executable(emulatord.power
        ${EMULATOR_DIR}/emulatord_power.cpp 
        ${EMULATOR_DIR}/power.cpp 
        )
target_link_libraries(emulatord.power
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# slit
# ----------------------------------------------------------------------------

add_executable(emulatord.slit
        ${EMULATOR_DIR}/emulatord_slit.cpp 
        ${EMULATOR_DIR}/slit.cpp 
        )
target_link_libraries(emulatord.slit
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# tcs
# ----------------------------------------------------------------------------

find_path( TCS_CONSTANTS "tcs_constants.h" PATHS ${PROJECT_BASE_DIR}/common )

add_executable(emulatord.tcs
        ${EMULATOR_DIR}/emulatord_tcs.cpp 
        ${EMULATOR_DIR}/tcs.cpp 
        ${TCS_CONSTANTS}
        ${PYTHON_DEV}
        )
target_link_libraries(emulatord.tcs
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        ${PYTHON_LIB}
        )

# -- Port Definitions ----------------------------------------------------------
#
# portdefs is the master list of ports for NGPS and is used to generate the
# individual .cfg files to avoid conflicts across multiple files, by keeping
# them all in one place..
#
INCLUDE( ${PROJECT_BASE_DIR}/Config/portdefs )
set ( ANDORSERVER_CFG_COMMENT "This file is automatically generated. Manual edits may be overwritten." )
CONFIGURE_FILE( ${PROJECT_BASE_DIR}/Config/andorserver.cfg.in ${PROJECT_BASE_DIR}/Config/andorserver.cfg )

