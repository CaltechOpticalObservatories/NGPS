/** ---------------------------------------------------------------------------
 * @file     build_date.h
 * @brief    definitions for preprocessor build date and time
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */
#pragma once

#include <string>
#include <ctime>
#include <sys/stat.h>

/// git commit this binary was built from, with a "-dirty" marker when the tree
/// carried uncommitted changes. Defined in provenance.cpp, which is the only
/// translation unit that includes the generated version.h, so that a change of
/// hash recompiles one file instead of everything including this header.
///
extern const std::string GIT_HASH_STR;

#define BUILD_DATE __DATE__  ///< preprocessor build date
#define BUILD_TIME __TIME__  ///< preprocessor build time


/***** get_build_time *********************************************************/
/**
 * @brief      returns the time that the running executable was built
 * @details    This uses the mtime of the running binary, which is when it was
 *             linked. That is more accurate than BUILD_DATE and BUILD_TIME,
 *             which are per-translation-unit and go stale whenever a header or
 *             a linked library changes without the main source being rebuilt.
 * @return     build time as YYYY-MM-DDThh:mm:ss UTC, empty if unavailable
 *
 */
inline std::string get_build_time() {
  struct stat st;
  struct tm   tmbuf;
  char        timestr[32];

  if ( stat( "/proc/self/exe", &st ) != 0 ) return "";

  if ( gmtime_r( &st.st_mtime, &tmbuf ) == nullptr ) return "";

  if ( strftime( timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S", &tmbuf ) == 0 ) return "";

  return std::string( timestr );
}
/***** get_build_time *********************************************************/
