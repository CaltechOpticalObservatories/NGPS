/** ---------------------------------------------------------------------------
 * @file     provenance.cpp
 * @brief    defines the build provenance of this binary
 * @details  This is deliberately the only translation unit that includes the
 *           generated version.h. version.h is regenerated on every build so
 *           that the "-dirty" marker reflects the tree at the moment of the
 *           build, and confining it here means a change recompiles only this
 *           file rather than everything that includes build_date.h.
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */
#include "build_date.h"
#include "version.h"

const std::string GIT_HASH_STR = GIT_COMMIT_HASH;
