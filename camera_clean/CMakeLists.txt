# ----------------------------------------------------------------------------
# @file    camera_clean/CMakeLists.txt
# @brief   input to the CMake build system for building camera_clean daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( CAMERAD_DIR ${PROJECT_BASE_DIR}/camera_clean )

set( CMAKE_CXX_STANDARD 17 )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )
link_directories( ${PROJECT_BASE_DIR}/lib )

# ----------------------------------------------------------------------------
# AstroCam ARC-64/66 PCI/e
# ----------------------------------------------------------------------------
message (STATUS "building for AstroCam GenIII PCI/PCIe")
add_definitions (-DCONTROLLER_ASTROCAM)
set (INTERFACE_TARGET astrocam)
set (ARCAPI_DIR "${PROJECT_BASE_DIR}/ARC")
find_path (ARCAPI_BASE "CArcBase.h" PATHS ${ARCAPI_DIR}/CArcBase/inc)
find_path (ARCAPI_FITS "CArcFitsFile.h" PATHS ${ARCAPI_DIR}/CArcFitsFile/inc)
find_path (ARCAPI_CFITS "CArcFitsFile.h" PATHS ${ARCAPI_DIR}/CArcFitsFile/inc)
find_path (ARCAPI_DEVICE "CArcDevice.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
find_path (ARCAPI_PCI "CArcPCI.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
find_path (ARCAPI_EXPIFACE "CExpIFace.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
find_path (ARCAPI_CONIFACE "CConIFace.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
find_path (ARCAPI_DEFS "ArcDefs.h" PATHS ${ARCAPI_DIR}/CArcDevice/inc)
add_subdirectory(${PROJECT_BASE_DIR}/ARC ${CMAKE_BINARY_DIR}/ARC)

# ----------------------------------------------------------------------------
#   INTERFACE_SOURCES    list of sources required for target interface
# ----------------------------------------------------------------------------
set (INTERFACE_SOURCES
    ${CAMERAD_DIR}/astrocam_interface.cpp
    ${CAMERAD_DIR}/astrocam_controller.cpp
    )

# ----------------------------------------------------------------------------
#   INTERFACE_INCLUDES  list of include paths for target interface
# ----------------------------------------------------------------------------
set (INTERFACE_INCLUDES
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
#   INTERFACE_LIBS  list of external libraries for target interface
# ----------------------------------------------------------------------------
set (INTERFACE_LIBS
    CArcBase
    CArcDevice
    CArcFitsFile
    )

# ----------------------------------------------------------------------------
# Camera Interface Base
# ----------------------------------------------------------------------------
list (APPEND INTERFACE_SOURCES
  ${CAMERAD_DIR}/camera_interface.cpp
  )
add_library(${INTERFACE_TARGET} ${INTERFACE_SOURCES})
target_link_libraries(${INTERFACE_TARGET}
                      common
                      )
target_include_directories(${INTERFACE_TARGET} PUBLIC ${INTERFACE_INCLUDES})

# ----------------------------------------------------------------------------
# External libraries, such as FITS, etc.
# ----------------------------------------------------------------------------

# FITS
#
find_library(CCFITS_LIB CCfits NAMES libCCfits PATHS /usr/local/lib)
find_library(CFITS_LIB cfitsio NAMES libcfitsio  PATHS /usr/local/lib)

# ZeroMQ
#
find_library( ZMQPP_LIB zmqpp NAMES libzmqpp PATHS /usr/local/lib )
find_library( ZMQ_LIB zmq NAMES libzmq PATHS /usr/local/lib )

# OpenCV
#
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Boost
#
find_package(Boost REQUIRED COMPONENTS thread chrono)
include_directories(${BOOST_INCLUDES})

find_package(Threads)

# ----------------------------------------------------------------------------
# build the camera daemon
# ----------------------------------------------------------------------------
add_executable(camera_clean
  ${CAMERAD_DIR}/camerad.cpp
  ${CAMERAD_DIR}/camera_server.cpp
  )

# ----------------------------------------------------------------------------
# link everything
# ----------------------------------------------------------------------------
target_link_libraries(camera_clean
                      network
                      utilities
                      logentry
                      ${INTERFACE_TARGET}
                      ${INTERFACE_LIBS}
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${CCFITS_LIB}
                      ${CFITS_LIB}
                      ${OpenCV_LIBS}
                      ${BOOST_INCLUDES}
                      Boost::thread
                      Boost::chrono
                      ${ZMQPP_LIB}
                      ${ZMQ_LIB}
                     )

