/**
 * @file    andor_emulator.h
 * @brief   header file for Andor emulator
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <cpython.h>

#define PYTHON_PATH "/home/developer/Software/Python:/home/developer/Software/Python/acam_skyinfo"
#define PYTHON_SKYSIM_MODULE "skysim"
#define PYTHON_SKYSIM_FUNCTION "simFromHeader"

/***** Andor ******************************************************************/
/**
 * @namespace  Andor
 * @brief      namespace for Andor cameras
 *
 */
namespace Andor {

  /***** Andor::SkySim ********************************************************/
  /**
   * @brief     used to generate a skysim image
   * @details   constructs a CPython object for the skysim module
   *
   */
  class SkySim {
    private:
      bool python_initialized;

    public:
      SkySim();

      CPython::CPyInstance py_instance;

      PyObject* pSkySimModule;

      long generate_image( const std::string_view &headerfile, const std::string_view &outputfile, const double exptime );
  };
  /***** Andor::SkySim ********************************************************/
}
