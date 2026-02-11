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

message( STATUS "CMAKE_BINARY_DIR" ${CMAKE_BINARY_DIR} )

# On macOS, default to the host architecture unless explicitly overridden.
# This avoids stale mixed-arch object files when switching between arm64/x86_64.
if ( APPLE AND NOT CMAKE_OSX_ARCHITECTURES )
  set( CMAKE_OSX_ARCHITECTURES "${CMAKE_SYSTEM_PROCESSOR}" CACHE STRING
       "Build architecture for macOS targets" FORCE )
endif()

# ZeroMQ/zmqpp library search paths must match the target architecture on macOS.
# Intel builds generally use /usr/local (Homebrew under Rosetta), while Apple
# Silicon builds use /opt/homebrew.
set( NGPS_ZMQ_FIND_MODE "" )
if ( APPLE )
  set( NGPS_ZMQ_FIND_MODE NO_DEFAULT_PATH )
  set( _ngps_target_arch "${CMAKE_OSX_ARCHITECTURES}" )
  if ( NOT _ngps_target_arch )
    set( _ngps_target_arch "${CMAKE_SYSTEM_PROCESSOR}" )
  endif()

  string( FIND "${_ngps_target_arch}" "x86_64" _ngps_has_x86 )
  string( FIND "${_ngps_target_arch}" "arm64" _ngps_has_arm )

  if ( _ngps_has_x86 GREATER -1 AND _ngps_has_arm EQUAL -1 )
    set( NGPS_ZMQ_SEARCH_PATHS
         /usr/local/lib
         /usr/local/opt/zeromq/lib
         /usr/local/opt/zeromq@4.3/lib
         /usr/local/opt/zmqpp/lib
         /usr/local/opt/zmqpp@4.2.0/lib )
  elseif ( _ngps_has_arm GREATER -1 AND _ngps_has_x86 EQUAL -1 )
    set( NGPS_ZMQ_SEARCH_PATHS
         /opt/homebrew/lib
         /opt/homebrew/opt/zeromq/lib
         /opt/homebrew/opt/zeromq@4.3/lib
         /opt/homebrew/opt/zmqpp/lib
         /opt/homebrew/opt/zmqpp@4.2.0/lib )
  else()
    set( NGPS_ZMQ_SEARCH_PATHS
         /usr/local/lib
         /usr/local/opt/zeromq/lib
         /usr/local/opt/zeromq@4.3/lib
         /usr/local/opt/zmqpp/lib
         /usr/local/opt/zmqpp@4.2.0/lib
         /opt/homebrew/lib
         /opt/homebrew/opt/zeromq/lib
         /opt/homebrew/opt/zeromq@4.3/lib
         /opt/homebrew/opt/zmqpp/lib
         /opt/homebrew/opt/zmqpp@4.2.0/lib )
  endif()

  # If architecture changed since the last configure, cached ZMQ library paths
  # can point at the wrong Homebrew prefix. Clear those cache entries so each
  # subdirectory can re-discover libraries for the current target arch.
  set( _ngps_disallow_prefix "" )
  if ( _ngps_has_x86 GREATER -1 AND _ngps_has_arm EQUAL -1 )
    set( _ngps_disallow_prefix "/opt/homebrew/" )
  elseif ( _ngps_has_arm GREATER -1 AND _ngps_has_x86 EQUAL -1 )
    set( _ngps_disallow_prefix "/usr/local/" )
  endif()
  if ( _ngps_disallow_prefix )
    if ( DEFINED ZMQ_LIB )
      string( FIND "${ZMQ_LIB}" "${_ngps_disallow_prefix}" _ngps_bad_zmq_pos )
      if ( _ngps_bad_zmq_pos EQUAL 0 )
        unset( ZMQ_LIB CACHE )
      endif()
    endif()
    if ( DEFINED ZMQPP_LIB )
      string( FIND "${ZMQPP_LIB}" "${_ngps_disallow_prefix}" _ngps_bad_zmqpp_pos )
      if ( _ngps_bad_zmqpp_pos EQUAL 0 )
        unset( ZMQPP_LIB CACHE )
      endif()
    endif()
  endif()
else()
  set( NGPS_ZMQ_SEARCH_PATHS
       /usr/local/lib
       /usr/local/opt/zeromq/lib
       /usr/local/opt/zeromq@4.3/lib
       /usr/local/opt/zmqpp/lib
       /usr/local/opt/zmqpp@4.2.0/lib )
endif()

set( NGPS_ZMQ_SEARCH_PATHS "${NGPS_ZMQ_SEARCH_PATHS}" CACHE STRING
     "Search paths for ZeroMQ and zmqpp libraries" )

# Resolve ZeroMQ libraries once at the top level so architecture mismatches
# fail early with a clear message instead of a linker-time symbol error.
find_library( ZMQPP_LIB NAMES zmqpp libzmqpp
              PATHS ${NGPS_ZMQ_SEARCH_PATHS} ${NGPS_ZMQ_FIND_MODE} )
find_library( ZMQ_LIB NAMES zmq libzmq
              PATHS ${NGPS_ZMQ_SEARCH_PATHS} ${NGPS_ZMQ_FIND_MODE} )

if ( NOT ZMQPP_LIB OR NOT ZMQ_LIB )
  if ( APPLE )
    message( FATAL_ERROR
             "Could not find architecture-compatible ZeroMQ libraries.\n"
             "  Target architecture: ${_ngps_target_arch}\n"
             "  Looked in: ${NGPS_ZMQ_SEARCH_PATHS}\n"
             "Install matching-arch zeromq/zmqpp under the correct Homebrew prefix "
             "(/usr/local for x86_64, /opt/homebrew for arm64), then re-run cmake." )
  else()
    message( FATAL_ERROR
             "Could not find ZeroMQ libraries (zmq and zmqpp).\n"
             "  Looked in: ${NGPS_ZMQ_SEARCH_PATHS}" )
  endif()
endif()

find_path( JSON_INCLUDE_DIR NAMES nlohmann/json.hpp
           PATHS /opt/homebrew/include /usr/local/include /usr/include )
if ( JSON_INCLUDE_DIR )
  include_directories( ${JSON_INCLUDE_DIR} )
else()
  message( WARNING "json.hpp not found; install nlohmann-json or set include path" )
endif()

option( NGPS_MINIMAL "Build minimal subset (utils, common, sequencerd)" OFF )
option( NGPS_WITH_ANDOR "Build Andor SDK-based components (andor/acamd/slicecamd)" ON )
option( NGPS_WITH_ANDOR_SDK "Build Andor SDK support (requires libandor and headers)" ON )
option( NGPS_WITH_ARC "Build ARC controller components" ON )

if ( NOT NGPS_WITH_ANDOR_SDK )
  include_directories( ${PROJECT_BASE_DIR}/stubs/andor )
  add_definitions( -DNGPS_NO_ANDOR_SDK )
endif()

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
if ( NGPS_MINIMAL )
  add_subdirectory( ${PROJECT_BASE_DIR}/utils )
  add_subdirectory( ${PROJECT_BASE_DIR}/common )
  add_subdirectory( ${PROJECT_BASE_DIR}/sequencerd )
else()
  if ( NGPS_WITH_ANDOR )
    add_subdirectory( ${PROJECT_BASE_DIR}/Andor )
    add_subdirectory( ${PROJECT_BASE_DIR}/acamd )
    add_subdirectory( ${PROJECT_BASE_DIR}/slicecamd )
  else()
    message( STATUS "Skipping Andor SDK components (andor/acamd/slicecamd)" )
  endif()
  add_subdirectory( ${PROJECT_BASE_DIR}/utils )
  add_subdirectory( ${PROJECT_BASE_DIR}/common )
  if ( NGPS_WITH_ARC )
    add_subdirectory( ${PROJECT_BASE_DIR}/camerad )
  else()
    message( STATUS "Skipping camerad (requires ARC)" )
  endif()
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
  if ( NGPS_WITH_ARC )
    add_subdirectory( ${PROJECT_BASE_DIR}/ARC )
  else()
    message( STATUS "Skipping ARC components" )
  endif()
  add_subdirectory( ${PROJECT_BASE_DIR}/LKS )
endif()
