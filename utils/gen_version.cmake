# ----------------------------------------------------------------------------
# @file    utils/gen_version.cmake
# @brief   regenerates version.h with the current git provenance
# @details Run at configure time and again on every build, so that the hash
#          and the "-dirty" marker describe the tree as it was when the build
#          ran. Editing a file does not move HEAD, so a configure-time-only
#          check would report a stale clean/dirty state.
#
#          Expects SRC_DIR, IN_FILE, TMP_FILE and OUT_FILE to be passed in
#          with -D. Writes through TMP_FILE so that copy_if_different can
#          leave OUT_FILE alone when nothing changed, avoiding a rebuild.
# ----------------------------------------------------------------------------

execute_process( COMMAND git rev-parse --short HEAD
                 WORKING_DIRECTORY ${SRC_DIR}
                 OUTPUT_VARIABLE GIT_COMMIT_HASH
                 OUTPUT_STRIP_TRAILING_WHITESPACE
                 RESULT_VARIABLE GIT_RESULT
                 ERROR_QUIET )

if ( NOT GIT_RESULT EQUAL 0 OR GIT_COMMIT_HASH STREQUAL "" )
  set( GIT_COMMIT_HASH "unknown" )
else()

  # A hash alone would name a commit that does not describe what was built,
  # so mark it when the tree carries uncommitted changes. This does not say
  # what changed, only that the commit cannot reproduce this binary.
  #
  execute_process( COMMAND git diff --quiet HEAD
                   WORKING_DIRECTORY ${SRC_DIR}
                   RESULT_VARIABLE GIT_TREE_DIRTY )

  if ( NOT GIT_TREE_DIRTY EQUAL 0 )
    set( GIT_COMMIT_HASH "${GIT_COMMIT_HASH}-dirty" )
  endif()

endif()

CONFIGURE_FILE( ${IN_FILE} ${TMP_FILE} )

execute_process( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TMP_FILE} ${OUT_FILE} )
