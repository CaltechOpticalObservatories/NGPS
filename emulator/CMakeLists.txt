# ----------------------------------------------------------------------------
# emulator/CMakeLists.txt
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( EMULATOR_DIR ${PROJECT_BASE_DIR}/emulator )

include_directories( ${EMULATOR_DIR} )
include_directories( ${PROJECT_BASE_DIR}/utils )
link_directories( ${PROJECT_BASE_DIR}/lib )

add_executable(emulatord.calib
        ${EMULATOR_DIR}/emulatord_calib.cpp 
        ${EMULATOR_DIR}/calib.cpp 
        )
target_link_libraries(emulatord.calib
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

add_executable(emulatord.flexure
        ${EMULATOR_DIR}/emulatord_flexure.cpp 
        ${EMULATOR_DIR}/flexure.cpp 
        )
target_link_libraries(emulatord.flexure
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

add_executable(emulatord.slit
        ${EMULATOR_DIR}/emulatord_slit.cpp 
        ${EMULATOR_DIR}/slit.cpp 
        )
target_link_libraries(emulatord.slit
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )
