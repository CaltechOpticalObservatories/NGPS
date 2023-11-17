#!/bin/bash

OUTPUT_FILE=$1

# Get the current date and time
BUILD_DATE=$(date)

# Get the git commit hash (if you're using git)
GIT_HASH=$(git rev-parse HEAD)

# Calculate the hash of your source files (replace "source1.cpp" and "source2.cpp" with your actual source files)
#SOURCE_HASH=$(sum *.cpp *.h | awk '{ print $1" "$3 }')

# Generate a C++ source file with variables containing build information
echo "#include <iostream>" > ${OUTPUT_FILE}
echo "const char* BuildDate = \"$BUILD_DATE\";" >> ${OUTPUT_FILE}
echo "const char* GitHash = \"$GIT_HASH\";" >> ${OUTPUT_FILE}

# Create a CMake config file with the build information
#echo "set(BUILD_DATE \"$BUILD_DATE\" CACHE STRING \"build time\" FORCE)" > build_info.cmake
#echo "set(GIT_HASH \"$GIT_HASH\" CACHE STRING \"git commit hash\" FORCE)" >> build_info.cmake
#echo "set(SOURCE_HASH \"$SOURCE_HASH\" CACHE STRING \"Source code hash\" FORCE)" >> $2/build_info.cmake

#echo "set(BUILD_DATE_TIME \"build date $(date)\")" > ${OUTPUT_FILE}
#echo "set(GIT_HASH \"git hash $(git rev-parse HEAD)\")" >> ${OUTPUT_FILE}
