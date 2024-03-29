/**
 * @file    cpython.h
 * @brief   tools for using the C-Python API
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#ifdef WITH_THREAD
#include "pythread.h"
#endif

#ifndef CPYTHON_H
#define CPYTHON_H

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
/***** CPython ****************************************************************/
#endif
