# ----------------------------------------------------------------------------
# @file    camerad/CMakeLists.txt
# @brief   input to the CMake build system for building camera daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( CAMERA_DIR ${PROJECT_BASE_DIR}/camerad )

set( CMAKE_CXX_STANDARD 17 )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )
link_directories( ${PROJECT_BASE_DIR}/lib )

# ----------------------------------------------------------------------------
# AstroCam ARC-64/66 PCI/e interfaces
# ----------------------------------------------------------------------------
message( STATUS "compiling for AstroCam GenIII PCI/PCIe")
set( INTERFACE_TARGET astrocam)
set( ARCAPI_DIR "${PROJECT_BASE_DIR}/ARC")
add_definitions( -Wall -ansi -O1 -Wno-variadic-macros -ggdb )
add_definitions( -DASTROCAM)
add_definitions( -DARC66_PCIE)
find_path( ARCAPI_BASE "CArcBase.h" PATHS ${ARCAPI_DIR}/CArcBase/inc)
find_path( ARCAPI_FITS "CArcFitsFile.h" PATHS ${ARCAPI_DIR}/CArcFitsFile/inc)
find_path( ARCAPI_CFITS "CArcFitsFile.h" PATHS ${ARCAPI_DIR}/CArcFitsFile/inc)
find_path( ARCAPI_DEVICE "CArcDevice.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
find_path( ARCAPI_PCI "CArcPCI.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
find_path( ARCAPI_EXPIFACE "CExpIFace.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
find_path( ARCAPI_CONIFACE "CConIFace.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
find_path( ARCAPI_DEFS "ArcDefs.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)

# ----------------------------------------------------------------------------
#   INTERFACE_SOURCE    list of sources required for target interface
# ----------------------------------------------------------------------------
set(INTERFACE_SOURCE
        "${CAMERA_DIR}/astrocam.cpp"
        )
# ----------------------------------------------------------------------------
#   INTERFACE_INCLUDES  list of include paths for target interface
# ----------------------------------------------------------------------------
set(INTERFACE_INCLUDES
        "${ARC_INTERFACE}"
        "${ARCAPI_BASE}"
        "${ARCAPI_FITS}"
        "${ARCAPI_CFITS}"
        "${ARCAPI_DEVICE}"
        "${ARCAPI_PCI}"
        "${ARCAPI_EXPIFACE}"
        "${ARCAPI_CONIFACE}"
        "${ARCAPI_DEFS}"
        )


# ----------------------------------------------------------------------------
# Add appropriate instrument library here.
# The user adds a "-DINSTR=<instr>" to the cmake command line.
# Currently supported <instr> names are: nirc2 generic
# ----------------------------------------------------------------------------

if ( NOT DEFINED INSTR )
    set( INSTR "undefined" )
endif()
if ( ${INSTR} STREQUAL "nirc2" )
    message( STATUS "building for NIRC2 instrument" )
    list( APPEND INTERFACE_SOURCE "${CAMERA_DIR}/nirc2.cpp" )
elseif( ${INSTR} STREQUAL "generic-arc" )
    message( STATUS "building for generic ARC controller" )
    list( APPEND INTERFACE_SOURCE "${CAMERA_DIR}/generic-arc.cpp" )
elseif( ${INSTR} STREQUAL "simulator-arc" )
    message( STATUS "building for simulated ARC controller" )
    list( APPEND INTERFACE_SOURCE "${CAMERA_DIR}/simulator-arc.cpp" )
elseif( ${INSTR} STREQUAL "generic-archon" )
    message( STATUS "building for generic Archon controller" )
    list( APPEND INTERFACE_SOURCE "${CAMERA_DIR}/generic-archon.cpp" )
elseif( ${INSTR} STREQUAL "undefined" )
    message( STATUS "no INSTR defined. options are:" )
    message( STATUS "cmake -DINSTR={nirc2,generic-archon,generic-arc,sim-arc} .." )
    list( APPEND INTERFACE_SOURCE "${CAMERA_DIR}/generic-archon.cpp" )
else()
    message( STATUS "unknown instrument " ${INSTR} )
endif()

# ----------------------------------------------------------------------------
# End interface setup
# ----------------------------------------------------------------------------

# Now add the defined interface target:
#
add_library(${INTERFACE_TARGET} ${INTERFACE_SOURCE})
target_include_directories(${INTERFACE_TARGET} PUBLIC ${INTERFACE_INCLUDES})

# ----------------------------------------------------------------------------
# External libraries, such as FITS, etc.
# ----------------------------------------------------------------------------

find_library(CCFITS_LIB CCfits NAMES libCCfits PATHS /usr/local/lib)
find_library(CFITS_LIB cfitsio NAMES libcfitsio  PATHS /usr/local/lib)

find_package(Threads)

add_library(camera STATIC
        ${CAMERA_DIR}/camera.cpp
        )

add_executable(camerad 
        ${CAMERA_DIR}/camerad.cpp 
        ${INTERFACE_INCLUDES}
        )

target_link_libraries(camerad
        camera
        network
        logentry
        common
        utilities
        md5
        CArcBase
        CArcDevice
        CArcFitsFile
        ${INTERFACE_TARGET}
        ${CMAKE_THREAD_LIBS_INIT}
        ${CCFITS_LIB}
        ${CFITS_LIB}
        )

# ----------------------------------------------------------------------------
# cURL is not used here, so it's not strictly required, but cfitsio can
# use it. If cfitsio was built with libcurl then it has to be linked here.
# If it's installed on the system then link it, but don't make it mandatory.
# ----------------------------------------------------------------------------
#
include(FindCURL)

if (CURL_FOUND STREQUAL "FALSE")
        message(STATUS "cURL was not found but may be needed by some systems if built into cfitsio.\n   If you experience undefined curl references during linking \n   then please your cURL installation.")
else()
        target_link_libraries(camerad ${CURL_LIBRARIES})
endif()


# -- Port Definitions ----------------------------------------------------------
#
# portdefs is the master list of ports for NGPS and is used to generate the
# individual .cfg files to avoid conflicts across multiple files, by keeping
# them all in one place..
#
INCLUDE( ${PROJECT_BASE_DIR}/Config/portdefs )
set ( CAMERAD_CFG_COMMENT "This file is automatically generated. Manual edits may be overwritten." )
CONFIGURE_FILE( ${PROJECT_BASE_DIR}/Config/camerad.cfg.in ${PROJECT_BASE_DIR}/Config/camerad.cfg )
