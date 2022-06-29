# ----------------------------------------------------------------------------
# utils/CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required( VERSION 3.12 )

set( PI_DIR ${PROJECT_BASE_DIR}/PI )

include_directories( ${PROJECT_BASE_DIR}/utils )

add_library( physik_instrumente STATIC
        ${PI_DIR}/pi.cpp
        )
