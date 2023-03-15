#include <Python.h>
#include <iostream>

class PyInstance {
  public:
    PyInstance(const char* pythonpath) { setenv( "PYTHONPATH", pythonpath, 1 ); Py_Initialize(); }
    ~PyInstance() { Py_Finalize(); }
};

class CPyObject {
  private:
    PyObject *p;
  public:
    CPyObject() : p(NULL) { }
    CPyObject( PyObject* pin ) : p(pin) { }
    ~CPyObject() { Release(); }
    void Release() { if (p) Py_DECREF(p); p=NULL; }
    operator PyObject*() { return p; }
    PyObject* operator ->() { return p; }
    PyObject* operator = (PyObject* pp) { p = pp; return p; }
    PyObject* getObject() { return p; }
    bool is() { return p ? true : false; }
};

class Foo {
  public:
    PyInstance instance{ "/home/developer/dhale/acam_skyinfo" };
    CPyObject pModuleName;
    CPyObject pModule;

    Foo() { std::cerr << "constructing Foo\n";
            this->pModuleName = PyUnicode_FromString( "astrometry" );
            this->pModule     = PyImport_Import( this->pModuleName );
            std::cerr << "constructed Foo\n";
          }
    ~Foo() { std::cerr <<"destroyed Foo\n";}

    int work() {
      if ( !this->pModule.is() ) { std::cerr << "bad module\n"; return 1; }

      CPyObject pFunc = PyObject_GetAttrString( this->pModule, "astrometry_cwrap" );
      const char* str = "/tmp/andor.fits";
      CPyObject pArgList = Py_BuildValue( "(s)", str );
      CPyObject pKeywords = PyDict_New();
      PyDict_SetItemString( pKeywords, "acquire", Py_True );
      PyDict_SetItemString( pKeywords, "foo", PyLong_FromLong(42) );
      PyDict_SetItemString( pKeywords, "bar", PyFloat_FromDouble(3.14) );

      if ( !pFunc.is() || !PyCallable_Check( pFunc ) ) { std::cerr << "not callable\n"; return 1; }

      CPyObject pReturn = PyObject_Call( pFunc, pArgList, pKeywords );

      if ( ! PyList_Check( pReturn.getObject() ) ) {
        return 1;
      }
      return 0;
    }
};

class Bar {
  public:
    Foo foo;
};

int main() {

//Foo foo;
  Bar bar;

  for ( int i=0; i<5; i++ ) {
//  int ret = foo.work();
    int ret = bar.foo.work();
    std::cerr << "ret=" << ret << "\n";
  }

  return 0;
}
