/** ---------------------------------------------------------------------------
 * @file    acam_interface.h
 * @brief   acam interface include
 * @details defines the classes used by the acam hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef ACAM_INTERFACE_H
#define ACAM_INTERFACE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "motion_interface.h"
#include "network.h"
#include "logentry.h"
#include "common.h"
#include "acamd_commands.h"
#include "atmcdLXd.h"
#include <cstdlib>
#include <iostream>
#include "camera.h"
#include "fits.h"
#include "config.h"

//#define PYTHON_PATH "/home/developer/Software/Python/acam_skyinfo"
//#define PYTHON_ASTROMETRY_MODULE "astrometry"
//#define PYTHON_ASTROMETRY_FUNCTION "astrometry_cwrap"

//#define PYTHON_PATH "/home/developer/Software/Python/acam_skyinfo:/home/developer/Software/Python/kapteyn-import"
#define PYTHON_PATH "/home/developer/Software/Python:/home/developer/Software/Python/acam_skyinfo"
#define PYTHON_ASTROMETRY_MODULE "astrometry"
#define PYTHON_ASTROMETRY_FUNCTION "astrometry_cwrap"

#define PYTHON_TEST_MODULE "test_py39"
#define PYTHON_TEST_FUNCTION "testfunc3"

#ifdef ANDORSIM
#include "andorsim.h"
#else
#include "andor.h"
#endif

/***** Acam *******************************************************************/
/**
 * @namespace Acam
 * @brief     namespace for acquisition and guide camera
 *
 */
namespace Acam {

  const std::string DAEMON_NAME = "acamd";       ///< when run as a daemon, this is my name

  class CPyInstance {
    private:
      char* restore_path;
      bool initialized;
    public:
      CPyInstance() { 
        Py_Initialize();
        initialized = ( Py_IsInitialized() ? true : false );
      }
      CPyInstance( const char* pythonpath ) { 
        this->restore_path = std::getenv( "PYTHONPATH" );
        setenv( "PYTHONPATH", pythonpath, 1 );
        Py_Initialize();
        initialized = ( Py_IsInitialized() ? true : false );
      }
      ~CPyInstance() { Py_Finalize(); setenv( "PYTHONPATH", this->restore_path, 1 ); initialized=false;}
      inline bool is_initialized() { return this->initialized; }

      /***** Acam::CPyInstance::pyobj_from_string *****************************/
      /**
       * @brief      create a PyObject of the correct type from a std::string
       * @param[in]  str_in  the input string
       * @param[out] obj     pointer to PyObject pointer (must be created outside this function)
       *
       */
      void pyobj_from_string( std::string str_in, PyObject** obj ) {
        std::size_t pos(0);

        // If the entire string is either exactly {true,false,True,False} then it's boolean
        //
        if ( str_in=="true"  || str_in=="True"  ) { *obj = Py_True;  return; }
        if ( str_in=="false" || str_in=="False" ) { *obj = Py_False; return; }

        // skip whitespaces
        //
        pos = str_in.find_first_not_of(' ');

        // check the significand, skip the sign if it exists
        //
        if ( str_in[pos] == '+' || str_in[pos] == '-' ) ++pos;

        // count the number of digits and number of decimal points
        //
        int n_nm, n_pt;
        for ( n_nm = 0, n_pt = 0; std::isdigit(str_in[pos]) || str_in[pos] == '.'; ++pos ) {
          str_in[pos] == '.' ? ++n_pt : ++n_nm;
        }
        if (n_pt>1 || n_nm<1 || pos<str_in.size()){   // no more than one point, no numbers, or a non-digit character
          const char* cstr = str_in.c_str();
          *obj = PyUnicode_FromString( cstr );
          return;
        }

        // skip the trailing whitespaces
        while (str_in[pos] == ' ') {
          ++pos;
        }

        try {
          if (pos == str_in.size()) {
            if (str_in.find(".") == std::string::npos) {  // all numbers and no decimals, it's an integer
              long num = std::stol( str_in );
              *obj = PyLong_FromLong( num );
              return;
            }
            else {                                        // otherwise numbers with a decimal, it's a float
              double num = std::stod( str_in );
              *obj = PyFloat_FromDouble( num );
              return;
            }
          }
          else {                                          // lastly, must be a string
            const char* cstr = str_in.c_str();
            *obj = PyUnicode_FromString( cstr );
            return;
          }
        }
        catch ( std::invalid_argument & ) {
          *obj = PyUnicode_FromString( "ERROR" );
          return;
        }
        catch ( std::out_of_range & ) {
          *obj = PyUnicode_FromString( "ERROR" );
          return;
        }


        // if not caught above then it's an error
        //
        *obj = PyUnicode_FromString( "ERROR" );
        return;
      }
      /***** Acam::CPyInstance::pyobj_from_string *****************************/
  };

/***
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
***/


  /***** Information **********************************************************/
  /**
   * @class   Information
   * @brief   information class for the ACAM camera
   * @details contains information about the Andor camera
   *
   */
  class Information {
    private:
    public:
  };
  /***** Information **********************************************************/


  /***** Acam::Camera *********************************************************/
  /**
   * @class  Camera
   * @brief  Camera class
   *
   * This class is used for communicating with the ACAM camera directly (which is an Andor)
   *
   */
  class Camera {
    private:

    public:
      Camera();
      ~Camera();

      uint16_t* image_data;

      Andor::Interface andor;     ///< create an Andor::Interface object for interfacing with the camera

      long open( std::string args );
      long close();
      long start_acquisition();
      long get_frame();
      long get_status();
  };
  /***** Acam::Camera *********************************************************/


  /***** Acam::CameraServer ***************************************************/
  /**
   * @class  CameraServer
   * @brief  CameraServer class
   *
   * This class is used for communicating with the ACAM camera via an external server
   *
   */
  class CameraServer {
    private:

      std::string name;        /// name for the external camera server
      std::string host;        /// host name for the external camera server
      int port;                /// port number for external camera server
      bool image_ready;
      std::string imagename;   /// fits filename of last acquired image
      std::string wcsfile;     /// fits filename of image containing updated WCS headers

    public:
      CameraServer();
      CameraServer( std::string host, int port );
      CameraServer( std::string name, std::string host, int port );
      ~CameraServer();

      long open();                                                   /// open a connection to the acam server
      inline long isopen() { return this->server.isconnected(); };   /// open a connection to the acam server
      long close();                                                  /// close the connection to the acam server
      long acquire( std::string wcsname, std::string &imagename );   /// acquire image from the acam server
      long coords( std::string coords_in );                          /// send coords to the acam server
      long send_command( std::string cmd, std::string &retstring );  /// send a command string to the camera server, accept reply
      long send_command( std::string cmd );                          /// send a command string to the camera server (no reply)

      Network::TcpSocket server;                                     /// TCP/IP socket object to communicate with the server
  };
  /***** Acam::CameraServer ***************************************************/


  /***** Acam::Astrometry *****************************************************/
  /**
   * @class  Astrometry
   * @brief  Astrometry class
   *
   * This class contains info for handling the Astrometry calculations.
   * Usage requires the C-Python API and the CPyInstance class defined in
   * cpython.h because the astrometry calculations are written in Python.
   * This class is the interface from C++ to the required Python modules.
   *
   */
  class Astrometry {
    private:
      char* restorePythonPath;
      std::string result;
      double ra, dec, pa;
      bool python_initialized;

    public:
      Astrometry();
      ~Astrometry();

      bool isacquire;
      inline bool is_initialized() { return this->python_initialized; };
      std::vector<std::string> solver_args;  /// contains list of optional solver args, "key1=val key2=val ... keyN=val"

      CPyInstance py_instance { PYTHON_PATH };
      PyObject* pModuleName;
      PyObject* pModule;

      PyObject* pTestModuleName;
      PyObject* pTestModule;

      inline std::string get_result() {
        std::stringstream result_str;
        result_str << result << " " << std::fixed << std::setprecision(6) << ra << " " << dec << " " << pa; 
        return result_str.str();
      }
      long solve( std::string imagename_in );
      long pytest( std::string imagename_in );
      long solverargs( std::string argsin, std::string &argsout );
      long compute_offset( std::string from, std::string to );
      long pytest();
      long cpytest();
  };
  /***** Acam::Astrometry *****************************************************/


  /***** Acam::Interface ******************************************************/
  /**
   * @class  Interface
   * @brief  interface class for acam
   *
   * This class defines the interface for the acam system and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      bool class_initialized;
      std::string imagename;
      std::string wcsname;

    public:
      std::string cameraserver_host;
      std::string motion_host;
      int cameraserver_port;
      int motion_port;

      Interface();
      ~Interface();

      inline std::string get_imagename() { return this->imagename; }
      inline std::string get_wcsname()   { return this->wcsname;   }

      inline void set_imagename( std::string name_in ) { this->imagename = name_in; return; }
      inline void set_wcsname( std::string name_in )   { this->wcsname = name_in;   return; }

      Information info;

      Common::Queue async;                     /// asynchronous message queue

      Camera camera;                           /// provides a direct interface to the Andor A&G camera

      CameraServer camera_server;              /// provides an interface to the Andor A&G camera via an external server

      MotionInterface motion;                  /// this object provides the interface to the filter and dust cover

      Astrometry astrometry;                   /// provides the interface to the Python astrometry packages

      FITS_file fits_file;                     /// instantiate a FITS container object

      long initialize_class();                 /// initialize the Interface class
      long test_image();                       ///
      long open( std::string args );           /// wrapper to open all acam-related hardware components
      bool isopen();                           /// wrapper for all acam-related hardware components
      long close();                            /// wrapper to open all acam-related hardware components
      long acquire();                          /// wrapper to acquire an Andor image
      long solve( std::string args, std::string &retstring );  /// wrapper for Astrometry::solve
      long exptime( std::string exptime_in, std::string &retstring );  /// wrapper to set exposure time

      inline void acquire_init() { imagename=""; wcsname=""; return; }

      long configure_interface( Config &config );
  };
  /***** Acam::Interface ******************************************************/

}
/***** Acam *******************************************************************/
#endif
