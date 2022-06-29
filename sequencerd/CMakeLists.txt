# ----------------------------------------------------------------------------
# sequencer/CMakeLists.txt
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( SEQUENCER_DIR ${PROJECT_BASE_DIR}/sequencerd )

include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )
link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(sequencerd
        ${SEQUENCER_DIR}/sequencerd.cpp 
        ${SEQUENCER_DIR}/sequencer_interface.cpp 
        ${SEQUENCER_DIR}/sequence.cpp 
        )

target_link_libraries(sequencerd
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        ${CCFITS_LIB}
        ${CFITS_LIB}
        )
