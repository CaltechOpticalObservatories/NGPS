/** ---------------------------------------------------------------------------
 * @file    slicecam_interface.h
 * @brief   slicecam interface include
 * @details defines the classes used by the slicecam hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <future>
#include <cmath>
#include <cpython.h>
#include "network.h"
#include "logentry.h"
#include "common.h"
#include "slicecamd_commands.h"
#include "atmcdLXd.h"
#include <cstdlib>
#include <iostream>
#include "slicecam_fits.h"
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

/***** Slicecam ***************************************************************/
/**
 * @namespace Slicecam
 * @brief     namespace for slicer cameras
 *
 */
namespace Slicecam {

  constexpr double PI = 3.14159265358979323846;

  class Interface;  // forward declaration

  /***** Slicecam::Camera *****************************************************/
  /**
   * @class  Camera
   * @brief  Camera class
   *
   * This class is used for communicating with the slicecam camera directly (which is an Andor)
   *
   */
  class Camera {
    private:
      uint16_t* image_data;
      int simsize;      /// for the sky simulator

    public:
      Camera() : image_data( nullptr ), simsize(1024) { };

      FITS_file fits_file;        /// instantiate a FITS container object
      FitsInfo  fitsinfo;

      std::map<std::string, std::shared_ptr<Andor::Interface>> andor;     ///< create a container for Andor::Interface objects

      inline void copy_info() { fits_file.copy_info( fitsinfo ); }
      inline void set_simsize( int val )     { if ( val > 0 ) this->simsize = val;  else throw std::out_of_range("simsize must be greater than 0");  }

      long emulator( std::string args, std::string &retstring );
      long open( std::string which, std::string args );
      long close();
      long get_frame();
      long write_frame( std::string source_file, std::string &outfile, const bool _tcs_online );
      long get_status();
      long bin( std::string args, std::string &retstring );
      long start_acquisition();
      long imflip( std::string args, std::string &retstring );
      long imrot( std::string args, std::string &retstring );
      long gain( std::string args, std::string &retstring );
      int gain();
      long exptime( std::string exptime_in, std::string &retstring );
      double exptime();
      long speed( std::string args, std::string &retstring );
      long temperature( std::string args, std::string &retstring );

      /**
       * This template function declares a function "execute_in_threads" which
       * accepts a function name of type Func and a variable argument list args.
       * This uses a trailing return type " -> " to set the return type of
       * execute_in_threads based on the return type of invoking func with the
       * provided arg list.
       *
       */
/****
      template <typename Func, typename... Args>
      auto execute_andor_thread( Func func, Args&&... args ) -> decltype( ( std::declval<Andor::Interface>().*func )( std::forward<Args>(args)... ) ) {

        // Determine the return type of func
        //
        using ReturnType = decltype( ( std::declval<Andor::Interface>().*func )( std::forward<Args>(args)... ) );
    
        // Vector to hold futures
        //
        std::vector<std::future<ReturnType>> futures;

        // Launch async tasks
        //
        for ( const auto &pair : this->andor ) {
          futures.push_back( std::async( std::launch::async, [pair, func, &args...]() -> ReturnType {
            return( pair.second.get()->*func )( std::forward<Args>(args)... );
          } ) );
        }

        // Aggregate results -- assumes the result can be OR'd
        //
        ReturnType result{};
        for ( auto &future : futures ) {
          result |= future.get();
        }

        return result;
      }

      void execute_in_threads( long (Interface::*func)(std::shared_ptr<Andor::Interface>)) {

        // Vector to store threads
        //
        std::vector<std::thread> threads;

        // Loop through the map and spawn a thread for each element
        //
        for ( const auto &pair : this->andor ) {
          threads.emplace_back( [this, func, param=pair.second]() { (this->*func)(param); } );
        }

        // Join all threads to wait for their completion
        //
        for ( auto &th : threads ) {
          if ( th.joinable() ) {
            th.join();
          }
        }
      }
****/
  };
  /***** Slicecam::Camera *****************************************************/


  /***** Slicecam::Telemetry **************************************************/
  /**
   * @class  Telemetry
   * @brief  interface class for slicecam
   *
   * This class defines the interface for the slicecam system and
   * contains the functions used to communicate with it.
   *
   */
  class Telemetry {
  };
  /***** Slicecam::Telemetry **************************************************/


  /***** Slicecam::GUIManager *************************************************/
  /**
   * @class  GUIManager
   * @brief  defines functions and settings for the display GUI
   *
   */
  class GUIManager {
    private:
      const std::string camera_name = "slicev";
      std::atomic<bool> update;  ///<! set if the menus need to be updated
      std::string push_settings; ///<! name of script to push settings to GUI
      std::string push_image;    ///<! name of script to push an image to GUI

    public:
      GUIManager() : update(false), exptime(NAN), gain(-1) { }

      // These are the GUIDER GUI settings
      //
      double exptime;
      int gain;

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
       * @brief      returns a formatted message of all gui settings
       * @details    This message is the return string to guideset command.
       * @return     string in form of <exptime> <gain>
       */
      std::string get_message_string() {
        std::stringstream message;
        if ( this->exptime < 0 ) message << "ERR"; else { message << std::fixed << std::setprecision(3) << this->exptime; }
        message << " ";
        if ( this->gain < 1 ) message << "ERR"; else { message << std::fixed << std::setprecision(3) << this->gain; }
        return message.str();
      }

      /**
       * @brief      calls the push_settings script with the formatted message string
       * @details    the script pushes the settings to the Guider GUI
       */
      void push_gui_settings() {
        std::string function = "Slicecam::GUIManager::push_gui_settings";
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
      void push_gui_image( std::string_view filename ) {
        std::string function = "Slicecam::GUIManager::push_gui_image";
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
  /***** Slicecam::GUIManager *************************************************/


  /***** Slicecam::Interface **************************************************/
  /**
   * @class   Interface
   * @brief   interface class for slicecam
   * @details This class defines the interface for the slicecam system and contains
   *          the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      std::mutex framegrab_mutex;
      std::atomic<bool> tcs_online;
      std::atomic<bool> err;
      std::string imagename;
      std::string wcsname;
      std::chrono::steady_clock::time_point wcsfix_time;
      std::chrono::steady_clock::time_point framegrab_time;

    public:

      std::atomic<bool> should_framegrab_run;  ///< set if framegrab loop should run
      std::atomic<bool> is_framegrab_running;  ///< set if framegrab loop is running

      GUIManager gui_manager;

      Interface() : tcs_online(false),
                    err(false),
                    should_framegrab_run(false),
                    is_framegrab_running(false) {
      }

      inline long read_error() { return( this->err.exchange( false ) ? ERROR : NO_ERROR ); };

      inline std::string get_imagename() { return this->imagename; }
      inline std::string get_wcsname()   { return this->wcsname;   }

      inline void set_imagename( std::string name_in ) { this->imagename = name_in; return; }
      inline void set_wcsname( std::string name_in )   { this->wcsname = name_in;   return; }

      Slicecam::FitsInfo fitsinfo;

      Common::Queue async;                     /// asynchronous message queue

      Camera camera;                           /// 

      FITS_file fits_file;                     /// instantiate a FITS container object

      TcsDaemonClient tcsd;                    /// for communicating with the TCS daemon, defined in ~/Software/tcsd/tcsd_client.h

      SkyInfo::FPOffsets fpoffsets;            /// for calling Python fpoffsets, defined in ~/Software/common/skyinfo.h

      long test_image();                       ///
      long open( std::string args, std::string &help);    /// wrapper to open all slicecams
      long isopen( std::string which, bool &state, std::string &help );     /// wrapper for slicecams
      bool isopen( std::string which );     /// wrapper for slicecams
      long close( std::string which, std::string &help );      /// wrapper to open all slicecams
      long tcs_init( std::string args, std::string &retstring );  /// initialize connection to TCS
      long framegrab( std::string args, std::string &retstring );    /// wrapper to control Andor frame grabbing
      long framegrab_fix( std::string args, std::string &retstring );    /// wrapper to control Andor frame grabbing
      long image_quality( std::string args, std::string &retstring );  /// wrapper for Astrometry::image_quality
      long put_on_slit( std::string args, std::string &retstring );  /// put target on slit
      long solve( std::string args, std::string &retstring );  /// wrapper for Astrometry::solve
      long gui_settings_control();          /// get gui settings and push to Guider GUI display
      long gui_settings_control( std::string args, std::string &retstring );  /// set or get and push to Guider GUI display
      long test( std::string args, std::string &retstring );

      long collect_header_info( std::shared_ptr<Andor::Interface> slicecam );

      inline void init_names() { imagename=""; wcsname=""; return; }  // TODO still needed?

      long configure_interface( Config &config );

      static void dothread_fpoffset( Slicecam::Interface &iface );
      void dothread_framegrab( const std::string whattodo, const std::string sourcefile );
      long acquire_one_threaded();
      long collect_header_info_threaded();
  };
  /***** Slicecam::Interface **************************************************/

}
/***** Slicecam ***************************************************************/
