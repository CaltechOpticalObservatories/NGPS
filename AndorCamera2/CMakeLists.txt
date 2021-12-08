# ----------------------------------------------------------------------------
# file:   AndorCamera2/CMakeLists.txt
# brief:  CMake input file for AndorCamera2
# author: David Hale <dhale@caltech.edu>
#
# This builds a required C library which the NGPS Andor Camera GUI needs.
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( ANDOR_DIR ${PROJECT_BASE_DIR}/AndorCamera2 )
set( LIBSRC ${ANDOR_DIR}/AndorCamera2.cpp )

include_directories( ${ANDOR_DIR} )

find_path( JNI "jni.h" PATHS "/usr/lib/jvm/jdk-16.0.2/include" )
find_path( JNI_MD "jni_md.h" PATHS "/usr/lib/jvm/jdk-16.0.2/include/linux/" )

set( INCLUDES "${JNI}" "${JNI_MD}")

find_library( andor "andor" NAMES "libandor.so" PATHS "/usr/local/lib" )

add_library( AndorCamera2 SHARED ${LIBSRC} ${INCLUDES} )

target_include_directories( AndorCamera2 PUBLIC ${INCLUDES} )

target_link_libraries( AndorCamera2 andor )
target_link_libraries( AndorCamera2 )
