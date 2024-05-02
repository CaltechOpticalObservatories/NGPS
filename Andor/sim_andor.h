/**
 * @file    sim_andor.h
 * @brief   
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef SIM_ANDOR_H
#define SIM_ANDOR_H

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
   * @brief     
   * @details   
   *
   */
  class SkySim {
    private:
      bool python_initialized;

    public:
      SkySim();

      CPython::CPyInstance py_instance;

      PyObject* pSkySimModule;

      long generate_image( std::string_view headerfile, std::string_view outputfile );
  };
  /***** Andor::SkySim ********************************************************/


  /***** Andor::CPyObject *****************************************************/
  /**
   * @brief     Pyhon C API PyObject wrapper
   * @details   This class manages the reference count of the PyObject pointer
   *            and provides access to the underlying Python object.
   *
   */
  class CPyObject {
    private:
      PyObject *p;
    public:
      CPyObject() : p(NULL) { }
      CPyObject( PyObject* pin ) : p(pin) { }
      ~CPyObject() { Release(); }

      PyObject* getObject() { return p; }
      PyObject* setObject(PyObject* _p) { return (p=_p); }
      void Release() { if (p) Py_DECREF(p); p=NULL; }
      PyObject* operator ->() { return p; }
      bool is() { return p ? true : false; }
      operator PyObject*() { return p; }
      PyObject* operator = (PyObject* pp) { p = pp; return p; }
      operator bool() { return p ? true : false; }
  };
  /***** Andor::CPyObject *****************************************************/

}
#endif
