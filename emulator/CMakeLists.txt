# ----------------------------------------------------------------------------
# emulator/CMakeLists.txt
# ----------------------------------------------------------------------------

cmake_minimum_required( VERSION 3.12 )

set( EMULATOR_DIR ${PROJECT_BASE_DIR}/emulator )

include_directories( ${EMULATOR_DIR} )
include_directories( ${PROJECT_BASE_DIR}/utils )
include_directories( ${PROJECT_BASE_DIR}/common )
link_directories( ${PROJECT_BASE_DIR}/lib )

# ----------------------------------------------------------------------------
# calib
# ----------------------------------------------------------------------------

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

# ----------------------------------------------------------------------------
# flexure
# ----------------------------------------------------------------------------

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

# ----------------------------------------------------------------------------
# power
# ----------------------------------------------------------------------------

add_executable(emulatord.power
        ${EMULATOR_DIR}/emulatord_power.cpp 
        ${EMULATOR_DIR}/power.cpp 
        )
target_link_libraries(emulatord.power
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )

# ----------------------------------------------------------------------------
# slit
# ----------------------------------------------------------------------------

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

# ----------------------------------------------------------------------------
# tcs
# ----------------------------------------------------------------------------

add_executable(emulatord.tcs
        ${EMULATOR_DIR}/emulatord_tcs.cpp 
        ${EMULATOR_DIR}/tcs.cpp 
        )
target_link_libraries(emulatord.tcs
        network
        logentry
        utilities
        ${CMAKE_THREAD_LIBS_INIT}
        )


