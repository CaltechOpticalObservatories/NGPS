# ----------------------------------------------------------------------------
# @file    camerad/CMakeLists.txt
# @brief   input to the CMake build system for building camera daemon
# @author  David Hale <dhale@caltech.edu>
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( CAMERA_DIR ${PROJECT_BASE_DIR}/camerad )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )
link_directories( ${PROJECT_BASE_DIR}/lib )

# ----------------------------------------------------------------------------
# AstroCam ARC-64/66 PCI/e interfaces
# ----------------------------------------------------------------------------
message( STATUS "compiling for AstroCam GenIII PCI/PCIe")
set( INTERFACE_TARGET astrocam)
set( ARCAPI_DIR "/opt/ARC_API/3.6")
add_definitions( -Wall -ansi -O2 -Wno-variadic-macros -std=c++17 -ggdb )
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
find_library(CARC_BASE "CArcBase3.6" NAMES "libCArcBase3.6" PATHS ${ARCAPI_DIR}/Release)
find_library(CARC_DEVICE "CArcDevice3.6" NAMES "libCArcDevice3.6.so" PATHS ${ARCAPI_DIR}/Release)
find_library(CARC_FITS "CArcFitsFile3.6" NAMES "libCArcFitsFile3.6.so" PATHS ${ARCAPI_DIR}/Release)

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
        ${INTERFACE_TARGET}
        ${CMAKE_THREAD_LIBS_INIT}
        ${CCFITS_LIB}
        ${CFITS_LIB}
        )

target_link_libraries(camerad ${CARC_BASE})
target_link_libraries(camerad ${CARC_DEVICE})
target_link_libraries(camerad ${CARC_FITS})

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

