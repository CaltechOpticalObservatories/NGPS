/**
 * @file    db_column_defs.h
 * @brief   binds JSON message keys to thermald DB column names
 *
 * Mirrors the pattern in fits_header_defs.h. Whenever a JSON message
 * key arriving via ZMQ pub/sub needs to be persisted to a column in
 * thermald.externaldata, add an entry here. The jkey value is
 * arbitrary (publisher and subscriber just need to agree on the
 * constant); the column field MUST match the actual DB schema.
 */
#pragma once

#include "message_keys.h"

namespace DbColumnDefs {

  /**
   * @struct Column
   * @brief  binds a JSON message key to a database column name
   */
  struct Column {
    const char* jkey;     ///< key in the inbound JSON message
    const char* column;   ///< database column name
  };

  const Column Columns[] = {
    { Key::Acamd::TANDOR.c_str(),       "TANDOR_ACAM"   },
    { Key::Slicecamd::TANDOR_L.c_str(), "TANDOR_SCAM_L" },
    { Key::Slicecamd::TANDOR_R.c_str(), "TANDOR_SCAM_R" }
  };

}
