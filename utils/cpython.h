/**
 * @file    cpython.h
 * @brief   tools for using the C-Python API
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#ifdef WITH_THREAD
#include "pythread.h"
#endif
#include <iostream>

/***** PyScope ****************************************************************/
/**
 * @namespace PyScope
 * @brief     establishes a scope guard template class
 *
 */
namespace PyScope {
    template<class F>
    class scope_guard {
        F func_;
        bool active_;
    public:
        scope_guard(F func) : func_(std::move(func)), active_(true) { }
        ~scope_guard() {
            if (active_) func_();
        }
    };
}
/***** PyScope ****************************************************************/


/** 
 * @def   PySCOPE
 * @brief use whenever a thread needs to use a Python call
 */
#define PySCOPE() \
    PyGILState_STATE gstate = PyGILState_Ensure(); \
    PyScope::scope_guard sggstate([&]() \
    { \
        PyGILState_Release(gstate); \
    })

/***** CPython ****************************************************************/
/**
 * @namespace CPython
 * @brief     namespace for C-Python tools
 *
 */
namespace CPython {

  /** @class   CPyInstance
   *  @brief   class for initializing a Python interpreter object
   */
  class CPyInstance {

    private:
      char* _restore_path;  /// save the PYTHONPATH env variable
      bool _initialized;    /// set when the Python interpreter is successfully initialized

      void initialize( const char* pythonpath ) {
        _restore_path = std::getenv( "PYTHONPATH" );    // Save the original PYTHONPATH

        if (pythonpath) {                               // Set PYTHONPATH if provided
          setenv( "PYTHONPATH", pythonpath, 1 );
        }

        Py_Initialize();                                // Initialize the Python interpreter

        PyGILState_STATE gstate = PyGILState_Ensure();  // Acquire the GIL

        _initialized = (Py_IsInitialized() ? true : false);

        PyGILState_Release( gstate );                   // Release the GIL
      }

    void finalize() {
      PyGILState_STATE gstate = PyGILState_Ensure();    // Acquire the GIL

      if ( _initialized && Py_IsInitialized() ) {
        setenv( "PYTHONPATH", (_restore_path ? _restore_path : "") , 1 );    // Restore the original PYTHONPATH
        Py_Finalize();                                  // Finalize the Python interpreter
        _initialized = false;
      }

      PyGILState_Release( gstate );                     // Release the GIL
    }

    public:

      /** @brief      class constructor
       *  @details    default constructor initializes interpreter
       */
      CPyInstance() { initialize( nullptr ); }

      /** @brief      class constructor
       *  @param[in]  pythonpath  path to set PYTHONPATH environment variable
       *  @details    sets PYTHONPATH and initializes interpreter
       */
      CPyInstance( const char* pythonpath ) { initialize( pythonpath ); }

      /** @brief      class de-constructor
       *  @details    restores original PYTHONPATH on destruction
       */
      ~CPyInstance() { finalize(); }

      /**
       *  @brief     writes to stderr when a Python error occurs
       *  @param[in] function  name of function where error occurred
       *  @details   PyErr_Print() should only be called when PyErr_Occurred()
       *             so this wraps that for safety.
       */
      void print_python_error( const std::string& function ) {
        if ( PyErr_Occurred() ) {
          std::cerr << "A Python error occurred in " << function << "\n";
          PyErr_Print();
        }
      }

      inline bool is_initialized() { return _initialized; }
  };

}
/***** CPython ****************************************************************/
