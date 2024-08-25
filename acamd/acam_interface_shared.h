/** ---------------------------------------------------------------------------
 * @file    acam_interface_shared.h
 * @brief   acam interface shared resources
 * @details defines things used by the acam hardware interface which might be
 *          shared with external libraries
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#pragma once

/***** Acam *******************************************************************/
/**
 * @namespace Acam
 * @brief     namespace for acquisition and guide camera
 *
 */
namespace Acam {

  const std::string POINTMODE_SLIT = "SLIT";
  const std::string POINTMODE_ACAM = "ACAM";

};
