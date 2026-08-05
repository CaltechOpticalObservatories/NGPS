# ----------------------------------------------------------------------------
# @file    CMakeLists.txt
# @brief   top-level CMake input file for NGPS
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

#add_definitions(-DLOGLEVEL_DEBUG)

cmake_minimum_required( VERSION 3.12 )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

project( NGPS )

set( PROJECT_BASE_DIR ${CMAKE_SOURCE_DIR} )

set( PROJECT_SRC_DIR ${PROJECT_BASE_DIR}/src )
set( PROJECT_INC_DIR ${PROJECT_BASE_DIR}/include )
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_BASE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${PROJECT_BASE_DIR}/lib )
set( CMAKE_C_COMPILER /usr/bin/g++ )
set( CMAKE_CXX_COMPILER /usr/bin/g++ )

find_package( Threads )

message( STATUS "CMAKE_BINARY_DIR" ${CMAKE_BINARY_DIR} )

# version.h carries the git provenance compiled into every daemon. It is
# generated into utils/ which is already on every include path.
#
set( GEN_VERSION_ARGS
     -DSRC_DIR=${CMAKE_SOURCE_DIR}
     -DIN_FILE=${PROJECT_BASE_DIR}/utils/version.h.in
     -DTMP_FILE=${CMAKE_BINARY_DIR}/version.h.tmp
     -DOUT_FILE=${PROJECT_BASE_DIR}/utils/version.h
     -P ${PROJECT_BASE_DIR}/utils/gen_version.cmake )

# Generate it now so that version.h exists for the first build.
#
execute_process( COMMAND ${CMAKE_COMMAND} ${GEN_VERSION_ARGS} )

# and again on every build. Editing a file moves neither HEAD nor any
# CMakeLists, so nothing would re-run configure and a configure-time check
# would report a stale clean/dirty state. The script leaves version.h alone
# when nothing changed, so this does not force a rebuild every time.
#
add_custom_target( update_version ALL
    COMMAND ${CMAKE_COMMAND} ${GEN_VERSION_ARGS}
    COMMENT "Checking git provenance"
)

# Here is everything to build
#
add_subdirectory( ${PROJECT_BASE_DIR}/Andor )
add_subdirectory( ${PROJECT_BASE_DIR}/acamd )
add_subdirectory( ${PROJECT_BASE_DIR}/slicecamd )
add_subdirectory( ${PROJECT_BASE_DIR}/utils )
add_subdirectory( ${PROJECT_BASE_DIR}/common )
add_subdirectory( ${PROJECT_BASE_DIR}/camerad )
add_subdirectory( ${PROJECT_BASE_DIR}/sequencerd )
add_subdirectory( ${PROJECT_BASE_DIR}/flexured )
add_subdirectory( ${PROJECT_BASE_DIR}/calibd )
add_subdirectory( ${PROJECT_BASE_DIR}/slitd )
add_subdirectory( ${PROJECT_BASE_DIR}/focusd )
add_subdirectory( ${PROJECT_BASE_DIR}/messaged )
add_subdirectory( ${PROJECT_BASE_DIR}/powerd )
add_subdirectory( ${PROJECT_BASE_DIR}/telemd )
add_subdirectory( ${PROJECT_BASE_DIR}/thermald )
add_subdirectory( ${PROJECT_BASE_DIR}/emulator )
add_subdirectory( ${PROJECT_BASE_DIR}/tcsd )
add_subdirectory( ${PROJECT_BASE_DIR}/PI )
add_subdirectory( ${PROJECT_BASE_DIR}/WTI )
add_subdirectory( ${PROJECT_BASE_DIR}/ARC )
add_subdirectory( ${PROJECT_BASE_DIR}/LKS )
