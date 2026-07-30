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

# Get the git commit hash and the build date so that the config files can use
# them. These are evaluated at configure time and stamped into the generated
# .cfg files, so a re-configure is what refreshes them.
#
execute_process( COMMAND git rev-parse --short HEAD
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                 OUTPUT_VARIABLE GIT_COMMIT_HASH
                 OUTPUT_STRIP_TRAILING_WHITESPACE )

# Mark the hash when the tree has uncommitted changes, otherwise the hash names
# a commit that does not describe what was actually built.
#
execute_process( COMMAND git diff --quiet HEAD
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                 RESULT_VARIABLE GIT_TREE_DIRTY )

if ( NOT GIT_TREE_DIRTY EQUAL 0 )
  set( GIT_COMMIT_HASH "${GIT_COMMIT_HASH}-dirty" )
endif()

string( TIMESTAMP PROJECT_BUILD_DATE "%Y-%m-%dT%H:%M:%S" UTC )

message( STATUS "GIT_COMMIT_HASH " ${GIT_COMMIT_HASH} " PROJECT_BUILD_DATE " ${PROJECT_BUILD_DATE} )

# Re-configure whenever the checked-out commit changes, so the stamps above
# cannot go stale. --git-path resolves correctly inside a linked worktree,
# where .git is a file rather than a directory.
#
execute_process( COMMAND git rev-parse --git-path HEAD
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                 OUTPUT_VARIABLE GIT_HEAD_FILE
                 OUTPUT_STRIP_TRAILING_WHITESPACE )

set_property( DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${GIT_HEAD_FILE} )

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
