/**
 * @file    andor_emulator.h
 * @brief   header file for Andor emulator
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <cpython.h>

/***** Andor ******************************************************************/
/**
 * @namespace  Andor
 * @brief      namespace for Andor cameras
 *
 */
namespace Andor {

  constexpr const char* PYTHON_PATH = "/home/developer/Software/Python:/home/developer/Software/Python/acam_skyinfo";
  constexpr const char* PYTHON_SKYSIM_MODULE = "skysim";
  constexpr const char* PYTHON_SKYSIM_FUNCTION = "simFromHeader";
  constexpr const char* PYTHON_SKYSIM_MULTI_FUNCTION = "simFromHeaderMulti";

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
      SkySim() : python_initialized(false) { }

      void initialize_python();

      CPython::CPyInstance py_instance { PYTHON_PATH };

      PyObject* pSkySimModule;

      long generate_image( const std::string_view &headerfile, const std::string_view &outputfile,
                           const float exptime, const bool ismex, const int simsize );

      void log_python_arguments(PyObject* pFunction, PyObject* pArgs, PyObject* pKwArgs);

      inline bool is_initialized() { return this->python_initialized; }
  };
  /***** Andor::SkySim ********************************************************/
}
