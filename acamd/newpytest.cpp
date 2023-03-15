#include <stdio.h>
#include <Python.h>
#include <iostream>
#define pythonpath "/home/developer/Software/Python/acam_skyinfo"

void check_obj( PyObject *obj, int level ) {
  if ( PyCell_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a cell\n"; return; }
  if ( PyCode_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a code object\n"; return; }
  if ( PyBool_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a bool\n"; return; }
  if ( PySlice_Check( obj ) )     { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a Slice\n"; return; }
  if ( PyUnicode_Check( obj ) )   { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a unicode\n"; return; }
  if ( PyFloat_Check( obj ) )     { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a float\n"; return; }
  if ( PyLong_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a long\n"; return; }
  if ( PyBytes_Check( obj ) )     { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a bytes\n"; return; }
  if ( PyComplex_Check( obj ) )   { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is complex\n"; return; }
  if ( PyByteArray_Check( obj ) ) { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a bytearray\n"; return; }
  if ( PyDict_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a Dict of size " << PyDict_Size( obj ) << "\n"; return; }
  if ( PyTuple_Check( obj ) )     { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a Tuple of size " << PyTuple_Size( obj ) << "\n"; return; }
  if ( PyList_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a List of size " << PyList_Size( obj ) << "\n"; return; }
  if ( PyMapping_Check( obj ) )   { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a mapping\n"; return; }
  return;
}

int main() {
  if ( setenv( "PYTHONPATH", pythonpath, 1 ) != 0 ) {
    std::cerr << "ERROR setting PYTHONPATH: " << errno << "\n";
  }
  Py_Initialize();

  PyObject* pName   = PyUnicode_FromString( "astrometry" );
  PyObject* pModule = PyImport_Import( pName );
  PyObject* pItem;
  PyObject* pArg;
  PyObject* pRet;

  if ( pModule != NULL ) {
    std::cerr << "module loaded\n";

    PyObject* pFunc = PyObject_GetAttrString( pModule, "astrometry_cwrap" );

    if ( pFunc && PyCallable_Check( pFunc ) ) {
      std::cerr << "calling function\n";

      pArg = PyUnicode_FromString( "/tmp/andor.fits" );
      pRet = PyObject_CallOneArg( pFunc, pArg );

      if ( pRet == NULL ) {
        PyErr_Print();
        return 1;
      }

      std::cerr << "Checking pReturn object...\n";
      check_obj( pRet, 0 );

      if ( PyList_Check( pRet ) ) {

        int num = PyList_Size(pRet);

        for ( int i=0; i<num; ++i ) {

          pItem = PyList_GetItem( pRet, i );

          std::cerr << "\tchecking item " << i << " of pReturn object...\n";
          check_obj( pItem, 1 );

          if ( PyUnicode_Check( pItem ) ) {
            int len = PyUnicode_GetLength( pItem );
            int kind = PyUnicode_KIND( pItem );
            void* data = PyUnicode_DATA( pItem );
            PyUnicode_READ( kind, data, len );
            std::cerr << "\t\tresult=" << (char*)data << "\n";
          }

          if ( PyFloat_Check( pItem ) ) {
            std::cerr << "\t\tresult=" << PyFloat_AsDouble( pItem ) << "\n";
          }
        }
      }

      Py_DECREF(pArg);
      Py_DECREF(pRet);
    }
    else {
      std::cerr << "ERROR: function not callable\n";
    }
    Py_DECREF(pFunc);
  }
  else {
    std::cerr << "ERROR: module not imported\n";
  }

  Py_DECREF(pName);
  Py_DECREF(pModule);
  return 0;
}
