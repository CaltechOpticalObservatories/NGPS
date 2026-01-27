# ----------------------------------------------------------------------------
# @file    CMakeLists.txt
# @brief   top-level CMake input file for NGPS
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

add_definitions(-DLOGLEVEL_DEBUG)

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

message( STATUS "PROJECT_BASE_DIR" ${PROJECT_BASE_DIR} )
message( STATUS "CMAKE_BINARY_DIR" ${CMAKE_BINARY_DIR} )

# Define the output file for the git commit hash, and
# touch it in case we're starting from scratch.
#
set(GIT_COMMIT_FILE "${CMAKE_BINARY_DIR}/git_commit.txt")
file( TOUCH ${GIT_COMMIT_FILE} )

# Create a custom target with a command that runs every time make is invoked.
# This creates the file which contains SET commands to set cmake variables.
#
add_custom_target(update_git_commit
    COMMAND ${CMAKE_COMMAND} -E remove ${GIT_COMMIT_FILE}
    COMMAND bash -c "echo -n set '\\( GIT_COMMIT_HASH ' " > ${GIT_COMMIT_FILE} &&
            bash -c "git rev-parse HEAD " | awk '{print $0}' | xargs echo -n >> ${GIT_COMMIT_FILE} &&
            bash -c "echo ' \\)'" >> ${GIT_COMMIT_FILE}
    COMMAND bash -c "echo -n set '\\( PROJECT_BUILD_DATE ' " >> ${GIT_COMMIT_FILE} &&
            bash -c "date +%Y-%m-%dT%H:%M:%S_%Z" | awk '{print $0}' | xargs echo -n >> ${GIT_COMMIT_FILE} &&
            bash -c "echo ' \\)'" >> ${GIT_COMMIT_FILE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Updating git commit hash file"
)

# include the git commit hash output file so that config files can use it
#
INCLUDE( ${GIT_COMMIT_FILE} )

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
add_subdirectory( ${PROJECT_BASE_DIR}/Galil )
