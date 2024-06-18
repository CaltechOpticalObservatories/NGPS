/** ---------------------------------------------------------------------------
 * @file    acam_interface.h
 * @brief   acam interface include
 * @details defines the classes used by the acam hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <cmath>
#include <cpython.h>
#include "motion_interface.h"
#include "network.h"
#include "logentry.h"
#include "common.h"
#include "acamd_commands.h"
#include "atmcdLXd.h"
#include <cstdlib>
#include <iostream>
#include "acam_fits.h"
#include "config.h"
#include "tcsd_commands.h"
#include "tcsd_client.h"
#include "skyinfo.h"

#define PYTHON_PATH "/home/developer/Software/Python:/home/developer/Software/Python/acam_skyinfo"
#define PYTHON_ASTROMETRY_MODULE "astrometry"
#define PYTHON_ASTROMETRY_FUNCTION "astrometry_cwrap"
#define PYTHON_IMAGEQUALITY_MODULE "image_quality"
#define PYTHON_IMAGEQUALITY_FUNCTION "image_quality_cwrap"

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

  constexpr double PI = 3.14159265358979323846;

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
      uint16_t* image_data;

    public:
      Camera() : image_data( nullptr ) { };

      FITS_file fits_file;        /// instantiate a FITS container object
      FitsInfo  fitsinfo;

      Andor::Interface andor;     ///< create an Andor::Interface object for interfacing with the camera

      inline void copy_info() { fits_file.copy_info( fitsinfo ); }

      long emulator( std::string args, std::string &retstring );
      long open( std::string args );
      long close();
      long start_acquisition();
      long get_frame();
      long write_frame( std::string source_file, std::string &outfile );
      long get_status();
      long bin( std::string args, std::string &retstring );
      long imflip( std::string args, std::string &retstring );
      long imrot( std::string args, std::string &retstring );
      long gain( std::string args, std::string &retstring );
      int gain();
      long exptime( std::string exptime_in, std::string &retstring );
      double exptime();
      long speed( std::string args, std::string &retstring );
      long temperature( std::string args, std::string &retstring );
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
      double seeing, seeing_zenith, extinction, background_med, background_std;
      bool python_initialized;

    public:
      Astrometry() : pAstrometryModule(nullptr), pQualityModule(nullptr) {  /// class constructor
        initialize_python();
      }

      void initialize_python();                          /// initializes the Python module

      bool isacquire;
      inline bool is_initialized() { return this->python_initialized; };
      std::vector<std::string> solver_args;              /// contains list of optional solver args, "key1=val key2=val ... keyN=val"

      CPython::CPyInstance py_instance { PYTHON_PATH };  /// initialize the Python interpreter

      PyObject* pAstrometryModule;                       /// astrometry
      PyObject* pQualityModule;                          /// image quality

      inline std::string get_result() {
        std::stringstream result_str;
        result_str << result << " " << std::fixed << std::setprecision(6) << ra << " " << dec << " " << pa; 
        return result_str.str();
      }

      void pyobj_from_string( std::string str_in, PyObject** obj );

      long image_quality( );
      long solve( std::string imagename_in );
      long solve( std::string imagename_in, std::vector<std::string> solverargs_in );
      long solverargs( std::string argsin, std::string &argsout );
      long compute_offset( std::string from, std::string to );
  };
  /***** Acam::Astrometry *****************************************************/


  /***** Acam::Telemetry ******************************************************/
  /**
   * @class  Telemetry
   * @brief  interface class for acam
   *
   * This class defines the interface for the acam system and
   * contains the functions used to communicate with it.
   *
   */
  class Telemetry {
  };
  /***** Acam::Telemetry ******************************************************/


  /***** Acam::GuideManager ***************************************************/
  /**
   * @class  GuideManager
   * @brief  defines functions and settings for the guider GUI
   *
   */
  class GuideManager {
    private:
      std::string camera_name = "guider";
      std::atomic<bool> update;  ///<! set if the menus need to be updated
      std::string push_settings; ///<! name of script to push settings to GUI
      std::string push_image;    ///<! name of script to push an image to GUI

    public:
      GuideManager() : update(false), exptime(NAN), gain(-1), filter("undef"), focus(NAN) { }

      // These are the GUIDER GUI settings
      //
      double exptime;
      int gain;
      std::string filter;  // Python needs an arg so filter can't be empty-initialize in constructor
      std::atomic<double> focus;

      // sets the private variable push_settings, call on config
      inline void set_push_settings( std::string sh ) { this->push_settings=sh; }

      // sets the private variable push_image, call on config
      inline void set_push_image( std::string sh ) { this->push_image=sh; }

      // sets the update flag true
      inline void set_update() { this->update.store( true ); return; }

      /**
       * @fn         get_update
       * @brief      returns the update flag then clears it
       * @return     boolean true|false
       */
      inline bool get_update() { return this->update.exchange( false ); }

      /**
       * @fn         get_message_string
       * @brief      returns a formatted message of all guider settings
       * @details    This message is the return string to guideset command.
       * @return     string in form of <exptime> <gain> <filter> <focus>
       */
      std::string get_message_string() {
        std::stringstream message;
        if ( this->exptime < 0 ) message << "ERR"; else { message << std::fixed << std::setprecision(3) << this->exptime; }
        message << " ";
        if ( this->gain < 1 ) message << "ERR"; else { message << std::fixed << std::setprecision(3) << this->gain; }
        message << " ";
        message << ( this->filter.empty() ? "undef" : this->filter );  // Python will need an arg so filter can't be empty
        message << " ";
        if ( std::isnan( this->focus ) ) message << "ERR"; else { message << std::fixed << std::setprecision(2) << this->focus; };
        return message.str();
      }

      /**
       * @brief      calls the push_settings script with the formatted message string
       * @details    the script pushes the settings to the Guider GUI
       */
      void push_guider_settings() {
        std::string function = "Acam::GuideManager::push_guider_settings";
        std::stringstream cmd;
        cmd << push_settings << " "
            << ( get_update() ? "true" : "false" ) << " "
            << get_message_string();

        if ( std::system( cmd.str().c_str() ) && errno!=ECHILD ) {
          logwrite( function, "ERROR updating GUI" );
        }

        return;
      }

      /**
       * @brief      calls the push_image script with the formatted message string
       * @details    the script pushes the indicated file to the Guider GUI display
       * @param[in]  filename  fits file to send
       */
      void push_guider_image( std::string_view filename ) {
        std::string function = "Acam::GuideManager::push_guider_image";
        std::stringstream cmd;
        cmd << push_image << " "
            << camera_name << " "
            << filename;

        if ( std::system( cmd.str().c_str() ) && errno!=ECHILD ) {
          logwrite( function, "ERROR pushing image to GUI" );
        }

        return;
      }
  };
  /***** Acam::GuideManager ***************************************************/


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
      std::atomic<bool> monitor_focus_thread_running;
      std::atomic<bool> acquire_thread_running;
      std::string imagename;
      std::string wcsname;
      std::chrono::steady_clock::time_point wcsfix_time;
      std::chrono::steady_clock::time_point acquire_time;

    public:
      std::string cameraserver_host;
      std::string motion_host;
      int cameraserver_port;
      int motion_port;

      GuideManager guide_manager;

      Interface() : monitor_focus_thread_running(false),
                    acquire_thread_running(false),
                    imagename(""), wcsname(""), cameraserver_host(""), motion_host(""), cameraserver_port(-1), motion_port(-1) { }

      inline std::string get_imagename() { return this->imagename; }
      inline std::string get_wcsname()   { return this->wcsname;   }

      inline void set_imagename( std::string name_in ) { this->imagename = name_in; return; }
      inline void set_wcsname( std::string name_in )   { this->wcsname = name_in;   return; }

      Acam::FitsInfo fitsinfo;

//    Telemetry telemetry;                     /// for collecting and writing telemetry data files

      Common::Queue async;                     /// asynchronous message queue

      Camera camera;                           /// 

      CameraServer camera_server;              /// provides an interface to the Andor A&G camera via an external server

      MotionInterface motion;                  /// this object provides the interface to the filter and dust cover

      Astrometry astrometry;                   /// provides the interface to the Python astrometry packages

      FITS_file fits_file;                     /// instantiate a FITS container object

      TcsDaemonClient tcsd;                    /// for communicating with the TCS daemon, defined in ~/Software/tcsd/tcsd_client.h

      SkyInfo::FPOffsets fpoffsets;            /// for calling Python fpoffsets, defined in ~/Software/common/skyinfo.h

      void initialize_python_objects();        /// provides interface to initialize all Python modules for objects in this class
      long initialize_class();                 /// initialize the Interface class
      long test_image();                       ///
      long open( std::string args, std::string &help);    /// wrapper to open all acam-related hardware components
      long isopen( std::string component, bool &state, std::string &help );     /// wrapper for acam-related hardware components
      long close( std::string component, std::string &help );      /// wrapper to open all acam-related hardware components
      long acquire( std::string args, std::string &retstring );    /// wrapper to acquire an Andor image
      long acquire_fix( std::string args, std::string &retstring );    /// wrapper to acquire an Andor image
      long image_quality( std::string args, std::string &retstring );  /// wrapper for Astrometry::image_quality
      long solve( std::string args, std::string &retstring );  /// wrapper for Astrometry::solve
      long guider_settings_control();          /// get guider settings and push to Guider GUI display
      long guider_settings_control( std::string args, std::string &retstring );  /// set or get and push to Guider GUI display
      long test( std::string args, std::string &retstring );

      long collect_header_info();
      long save_acquired();

      inline void acquire_init() { imagename=""; wcsname=""; return; }

      long configure_interface( Config &config );

      static void dothread_acquire( Acam::Interface &iface, const std::string whattodo, const std::string sourcefile );
      static void dothread_set_filter( Acam::Interface &iface, std::string filter_req );
      static void dothread_set_focus( Acam::Interface &iface, double focus_req );
      static void dothread_monitor_focus( Acam::Interface &iface );
  };
  /***** Acam::Interface ******************************************************/

}
/***** Acam *******************************************************************/
