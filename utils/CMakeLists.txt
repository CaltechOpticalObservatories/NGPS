# ----------------------------------------------------------------------------
# utils/CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.12)

set(PROJECT_UTILS_DIR ${PROJECT_BASE_DIR}/utils)

add_library(utilities STATIC
        ${PROJECT_UTILS_DIR}/utilities.cpp
        )

add_library(logentry STATIC
        ${PROJECT_UTILS_DIR}/logentry.cpp
        )

add_library(network STATIC
        ${PROJECT_UTILS_DIR}/network.cpp
        )