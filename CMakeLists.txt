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

set( PROJECT_BASE_DIR $ENV{PWD}/../ )

set( PROJECT_SRC_DIR ${PROJECT_BASE_DIR}/src )
set( PROJECT_INC_DIR ${PROJECT_BASE_DIR}/include )
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_BASE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${PROJECT_BASE_DIR}/lib )
set( CMAKE_C_COMPILER /usr/bin/g++ )
set( CMAKE_CXX_COMPILER /usr/bin/g++ )

find_package( Threads )

message( STATUS "CMAKE_BINARY_DIR" ${CMAKE_BINARY_DIR} )

# Get the git commit hash identifying the source that this build came from.
# It is compiled into every daemon via the generated version.h below, so that
# each binary can report the commit it was built from.
#
execute_process( COMMAND git rev-parse --short HEAD
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                 OUTPUT_VARIABLE GIT_COMMIT_HASH
                 OUTPUT_STRIP_TRAILING_WHITESPACE
                 RESULT_VARIABLE GIT_RESULT
                 ERROR_QUIET )

# git failing here would silently leave the hash empty,
# so make the failure visible instead
#
if ( NOT GIT_RESULT EQUAL 0 OR GIT_COMMIT_HASH STREQUAL "" )
  set( GIT_COMMIT_HASH "unknown" )
  message( WARNING "unable to determine git commit hash: reporting GIT_HASH=unknown" )
endif()

message( STATUS "GIT_COMMIT_HASH " ${GIT_COMMIT_HASH} )

# Re-configure whenever the checked-out commit changes, so the hash cannot go
# stale. --git-path resolves correctly inside a linked worktree, where .git is
# a file rather than a directory.
#
execute_process( COMMAND git rev-parse --git-path HEAD
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                 OUTPUT_VARIABLE GIT_HEAD_FILE
                 OUTPUT_STRIP_TRAILING_WHITESPACE
                 ERROR_QUIET )

if ( GIT_HEAD_FILE )
  set_property( DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${GIT_HEAD_FILE} )
endif()

# Generate version.h into utils/ which is already on every daemon's include
# path. It is written through a temporary so that copy_if_different can leave
# the real header untouched when the hash has not changed, which keeps an
# unrelated re-configure from forcing a rebuild of everything.
#
CONFIGURE_FILE( ${PROJECT_BASE_DIR}/utils/version.h.in
                ${CMAKE_BINARY_DIR}/version.h.tmp )

execute_process( COMMAND ${CMAKE_COMMAND} -E copy_if_different
                         ${CMAKE_BINARY_DIR}/version.h.tmp
                         ${PROJECT_BASE_DIR}/utils/version.h )

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
