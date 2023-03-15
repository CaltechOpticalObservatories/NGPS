#include <stdio.h>
#include <Python.h>
#include <iostream>
#define pythonpath "/home/developer/Software/Python/acam_skyinfo"

void check_obj( PyObject *obj, int level ) {
  if ( PyCell_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a cell\n"; }
  if ( PyCode_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a code object\n"; }
  if ( PyBool_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a bool\n"; }
  if ( PyList_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a List of size " << PyList_Size( obj ) << "\n"; }
  if ( PySlice_Check( obj ) )     { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a Slice\n"; }
  if ( PyMapping_Check( obj ) )   { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a mapping\n"; }
  if ( PyUnicode_Check( obj ) )   { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a unicode\n"; }
  if ( PyFloat_Check( obj ) )     { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a float\n"; }
  if ( PyLong_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a long\n"; }
  if ( PyBytes_Check( obj ) )     { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a bytes\n"; }
  if ( PyComplex_Check( obj ) )   { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is complex\n"; }
  if ( PyByteArray_Check( obj ) ) { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a bytearray\n"; }
  if ( PyDict_Check( obj ) )      { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a Dict of size " << PyDict_Size( obj ) << "\n"; }
  if ( PyTuple_Check( obj ) )     { for (int i=0; i<level; i++) std::cerr << "\t"; std::cerr << "obj is a Tuple of size " << PyTuple_Size( obj ) << "\n"; }
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
  PyObject* pItemItem;
  PyObject* pArg;
  PyObject* pRet;

  if ( pModule != NULL ) {
    std::cerr << "module loaded\n";

//  PyObject* pFunc = PyObject_GetAttrString( pModule, "astrometry_script" );
    PyObject* pFunc = PyObject_GetAttrString( pModule, "astrometry_cwrap" );

    if ( pFunc && PyCallable_Check( pFunc ) ) {
      std::cerr << "calling function\n";

      pArg = PyUnicode_FromString( "/tmp/andor.fits" );
      pRet = PyObject_CallOneArg( pFunc, pArg );

      if ( pRet == NULL ) {
        PyErr_Print();
        return 1;
      }

//    pItem = PyTuple_GET_ITEM( pRet, 0 );
//    std::cerr << "first item is" << ( PyBool_Check( pItem ) ? "" : " not" ) << " a bool\n";
//    Py_DECREF( pItem );

      std::cerr << "Checking pReturn object...\n";
      check_obj( pRet, 0 );

      if ( PyTuple_Check( pRet ) ) {

        int num = PyTuple_Size(pRet);

        for ( int i=0; i<num; ++i ) {

          pItem = PyTuple_GetItem( pRet, i );

          std::cerr << "\tchecking item " << i << " of pReturn object...\n";
          check_obj( pItem, 1 );

          if ( PyTuple_Check( pItem ) ) { 
            std::cerr << "item " << i << " of pRet is a Tuple of size " << PyTuple_Size( pItem ) << "\n";

            for ( int j=0; j< PyTuple_Size( pItem ); ++j ) {
              pItemItem = PyTuple_GetItem( pItem, j );
              std::cerr << "\t\tchecking subitem " << j << " of pReturn tuple " << i << "...\n";
              check_obj( pItemItem, 2 );
//            Py_DECREF( pItemItem );
            }
          }

          if ( PyDict_Check( pItem ) ) { 
            PyObject* resultkey = PyUnicode_InternFromString( "result" );
            PyObject* result    = PyDict_GetItem( pItem, resultkey );
            std::cerr << "\t\t\tchecking result\n";
            check_obj( result, 4 );

            int len = PyUnicode_GetLength( result );
            int kind = PyUnicode_KIND( result );
            void* data = PyUnicode_DATA( result );
            PyUnicode_READ( kind, data, len );
            std::cerr << "\t\t\t\tresult=" << (char*)data << "\n";


            PyObject* image_center_key = PyUnicode_InternFromString( "image_center" );
            PyObject* image_center     = PyDict_GetItem( pItem, image_center_key );
            std::cerr << "\t\t\tchecking image_center\n";
            check_obj( image_center, 4 );

            PyObject* pImageCoord;
            for ( int ii=0; ii < PyTuple_Size( image_center ); ++ii ) {
              pImageCoord = PyTuple_GetItem( image_center, ii );
              std::cerr << "\t\t\t\tchecking pImageCoord item " << ii << "\n";
              check_obj( pImageCoord, 5 );
              if ( PyMapping_Check( pImageCoord ) ) {
                std::cerr << "\t\t\t\t\tPyMappingSize of item " << ii << " is " << PyMapping_Size( pImageCoord ) << "\n";
                std::cerr << "\t\t\t\t\tPyMappingLength of item " << ii << " is " << PyMapping_Length( pImageCoord ) << "\n";
              }
            }

//          Py_DECREF(resultkey);
//          Py_DECREF(result);
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
