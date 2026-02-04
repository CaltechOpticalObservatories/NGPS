/**
 * @file    andor_emulator.h
 * @brief   header file for Andor emulator
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <cpython.h>
#include <cstdlib>
#include <map>
#include <string>

/***** Andor ******************************************************************/
/**
 * @namespace  Andor
 * @brief      namespace for Andor cameras
 *
 */
namespace Andor {

  constexpr const char* PYTHON_SKYSIM_MODULE = "skysim";
  constexpr const char* PYTHON_SKYSIM_FUNCTION = "simFromHeader";
  constexpr const char* PYTHON_SKYSIM_MULTI_FUNCTION = "simFromHeaderMulti";

  inline const char* python_path() {
    static std::string path;
    if ( path.empty() ) {
      const char* root = std::getenv( "NGPS_ROOT" );
      if ( root && *root ) {
        path = std::string( root ) + "/Python:" + std::string( root ) + "/Python/acam_skyinfo";
      }
      else {
        path = "/home/developer/Software/Python:/home/developer/Software/Python/acam_skyinfo";
      }
      const char* envpath = std::getenv( "PYTHONPATH" );
      if ( envpath && *envpath ) {
        path.append( ":" );
        path.append( envpath );
      }
    }
    return path.c_str();
  }

  /***** Andor::SkySim ********************************************************/
  /**
   * @brief     used to generate a skysim image
   * @details   constructs a CPython object for the skysim module
   *
   */
  class SkySim {
    private:
      bool python_initialized;
      std::map<std::string, std::string> sim_kwargs;

    public:
      SkySim() : python_initialized(false) { }

      void initialize_python();

      CPython::CPyInstance py_instance { python_path() };

      PyObject* pSkySimModule;

      long generate_image( const std::string_view &headerfile, const std::string_view &outputfile,
                           const float exptime, const bool ismex, const int simsize );

      inline void set_kwarg( const std::string &key, const std::string &value ) { this->sim_kwargs[key] = value; }
      inline void clear_kwargs() { this->sim_kwargs.clear(); }

      inline bool is_initialized() { return this->python_initialized; }
  };
  /***** Andor::SkySim ********************************************************/
}
