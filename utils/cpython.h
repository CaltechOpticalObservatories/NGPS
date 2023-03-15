/**
 * @file    cpython.h
 * @brief   tools for using the C-Python API
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include <Python.h>

#ifndef CPYTHON_H
#define CPYTHON_H

/** @namespace CPython
 *  @brief     namespace for C-Python tools
 */
namespace CPython {

  /** @class   CPyInstance
   *  @brief   class for initializing a Python interpreter object
   */
  class CPyInstance {

    private:
      char* restore_path;  /// save the PYTHONPATH env variable
      bool initialized;    /// set when the Python interpreter is successfully initialized

    public:

      /** @brief      class constructor
       *  @details    default constructor initializes interpreter
       */
      CPyInstance() {
        Py_Initialize();
        initialized = ( Py_IsInitialized() ? true : false );
      }

      /** @brief      class constructor
       *  @param[in]  pythonpath  path to set PYTHONPATH environment variable
       *  @details    sets PYTHONPATH and initializes interpreter
       */
      CPyInstance( const char* pythonpath ) {
        this->restore_path = std::getenv( "PYTHONPATH" );
        setenv( "PYTHONPATH", pythonpath, 1 );
        Py_Initialize();
        initialized = ( Py_IsInitialized() ? true : false );
      }

      /** @brief      class de-constructor
       *  @details    restores original PYTHONPATH on destruction
       */
      ~CPyInstance() { Py_Finalize(); setenv( "PYTHONPATH", this->restore_path, 1 ); initialized=false; }

      inline bool is_initialized() { return this->initialized; }
  };

}
#endif
