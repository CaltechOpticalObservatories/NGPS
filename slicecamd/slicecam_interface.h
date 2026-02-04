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
#include <map>
#include <cpython.h>
#include "network.h"
#include "logentry.h"
#include "common.h"
#include "slicecamd_commands.h"
#include "atmcdLXd.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include "slicecam_fits.h"
#include "config.h"
#include "tcsd_commands.h"
#include "acamd_commands.h"
#include "tcsd_client.h"
#include "skyinfo.h"

#define PYTHON_ASTROMETRY_MODULE "astrometry"
#define PYTHON_ASTROMETRY_FUNCTION "astrometry_cwrap"
#define PYTHON_IMAGEQUALITY_MODULE "image_quality"
#define PYTHON_IMAGEQUALITY_FUNCTION "image_quality_cwrap"

inline const char* python_path() {
  static std::string path;
  if ( path.empty() ) {
    const char* root = std::getenv( "NGPS_ROOT" );
    if ( root && *root ) {
      path = std::string( root ) + "/Python:" + std::string( root ) + "/Python/acam_skyinfo";
    }
    else {
      path = "/home/developer/Software/Python:/home/developer/Software/Python/acam_skyinfo";
    }
    const char* envpath = std::getenv( "PYTHONPATH" );
    if ( envpath && *envpath ) {
      path.append( ":" );
      path.append( envpath );
    }
  }
  return path.c_str();
}

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

  const std::string DAEMON_NAME = "slicecamd";       ///< when run as a daemon, this is my name

  constexpr std::string_view DEFAULT_IMAGENAME = "/tmp/slicecam.fits";

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
      std::map<at_32, at_32> handlemap;
      std::map<std::string, std::string> skysim_args;

    public:
      Camera() : image_data( nullptr ), simsize(1024) { };

      FITS_file fits_file;        /// instantiate a FITS container object
      FitsInfo  fitsinfo;

      std::mutex framegrab_mutex;

      std::map<std::string, std::unique_ptr<Andor::Interface>> andor;     ///< container for Andor::Interface objects

      std::map<std::string, Andor::DefaultValues> default_config;         ///< container to hold defaults for each camera

      inline void copy_info() { fits_file.copy_info( fitsinfo ); }
      inline void set_simsize( int val )     { if ( val > 0 ) this->simsize = val;  else throw std::out_of_range("simsize must be greater than 0");  }
      inline void set_skysim_arg( const std::string &key, const std::string &value ) {
        this->skysim_args[key] = value;
        for ( auto &entry : this->andor ) {
          entry.second->emulator.skysim.set_kwarg( key, value );
        }
      }
      inline void clear_skysim_args() {
        this->skysim_args.clear();
        for ( auto &entry : this->andor ) {
          entry.second->emulator.skysim.clear_kwargs();
        }
      }
      inline void apply_skysim_args() {
        if ( this->skysim_args.empty() ) return;
        for ( auto &entry : this->andor ) {
          for ( const auto &kv : this->skysim_args ) {
            entry.second->emulator.skysim.set_kwarg( kv.first, kv.second );
          }
        }
      }

      inline long init_handlemap() {
        this->handlemap.clear();
        return this->andor.begin()->second->get_handlemap( this->handlemap );
      }

      long emulator( std::string args, std::string &retstring );
      long open( std::string which, std::string args );
      long close();
      long get_frame();
      long write_frame( std::string source_file, std::string &outfile, const bool _tcs_online );
      long bin( const int hbin, const int vbin );
      long set_fan( std::string which, int mode );
      long imflip( std::string args, std::string &retstring );
      long imrot( std::string args, std::string &retstring );
      long set_gain( int &gain );
      long set_gain( std::string which, int &gain );
      long set_gain( int &&gain );
      long set_gain( std::string which, int &&gain );
      long set_exptime( float &val );
      long set_exptime( std::string which, float &val );
      long set_exptime( float &&val );
      long set_exptime( std::string which, float &&val );
      long speed( std::string args, std::string &retstring );
      long temperature( std::string args, std::string &retstring );
  };
  /***** Slicecam::Camera *****************************************************/


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
      GUIManager() : update(false), exptime(NAN), gain(-1), bin(-1), navg(NAN) { }

      // These are the GUIDER GUI settings
      //
      float exptime;
      int gain;
      int bin;
      float navg;

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
       * @return     string in form of <exptime> <gain> <bin>
       */
      std::string get_message_string() {
        std::stringstream message;
        if ( this->exptime < 0 ) message << "ERR"; else { message << std::fixed << std::setprecision(3) << this->exptime; }
        message << " ";
        if ( this->gain < 1 ) message << "ERR"; else { message << std::fixed << std::setprecision(3) << this->gain; }
        message << " ";
        if ( this->bin < 1 ) message << "x"; else { message << std::fixed << std::setprecision(3) << this->bin; }
        message << " ";
        if ( std::isnan(this->navg) ) message << "NaN"; else { message << std::fixed << std::setprecision(2) << this->navg; }
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

      void send_fifo_warning(const std::string &message) {
        const std::string fifo_name("/tmp/.slicev_warning.fifo");
        std::ofstream fifo(fifo_name);
        if (!fifo.is_open()) {
          logwrite("Slicecam::GUIManager::send_fifo_warning", "failed to open " + fifo_name + " for writing");
        }
        else {
          fifo << message << std::endl;
          fifo.close();
        }
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
      zmqpp::context context;
      std::mutex framegrab_mutex;
      std::atomic<bool> tcs_online;
      std::atomic<bool> err;
      std::string imagename;
      std::string wcsname;
      std::chrono::steady_clock::time_point wcsfix_time;
      std::chrono::steady_clock::time_point framegrab_time;
      std::mutex framegrab_mtx;
      std::condition_variable cv;

    public:
      std::unique_ptr<Common::PubSub> publisher;       ///< publisher object
      std::string publisher_address;                   ///< publish socket endpoint
      std::string publisher_topic;                     ///< my default topic for publishing
      std::unique_ptr<Common::PubSub> subscriber;      ///< subscriber object
      std::string subscriber_address;                  ///< subscribe socket endpoint
      std::vector<std::string> subscriber_topics;      ///< list of topics I subscribe to
      std::atomic<bool> is_subscriber_thread_running;  ///< is my subscriber thread running?
      std::atomic<bool> should_subscriber_thread_run;  ///< should my subscriber thread run?
      std::unordered_map<std::string,
                         std::function<void(const nlohmann::json&)>> topic_handlers;
                                                       ///< maps a handler function to each topic

      std::atomic<bool> should_framegrab_run;  ///< set if framegrab loop should run
      std::atomic<bool> is_framegrab_running;  ///< set if framegrab loop is running

      /** these are set by Interface::saveframes()
       */
      std::atomic<int> nsave_preserve_frames;  ///< number of frames to preserve (normally overwritten)
      std::atomic<int> nskip_preserve_frames;  ///< number of frames to skip before saving nsave... frames

      struct {
        std::string tcsname;
        bool is_tcs_open;
        double angle_scope;
        std::string ra_scope_hms;   // h:m:s
        std::string dec_scope_dms;  // d:m:s
        double ra_scope_h;          // decimal hours
        double dec_scope_d;         // decimal degrees
        double offsetra;
        double offsetdec;
        double az;
        double telfocus;
        double airmass;
        double slitoffset;
        double slitwidth;
      } telem;                      ///< holds my subscribed-to telemetry

      std::mutex snapshot_mtx;
      std::unordered_map<std::string, bool> snapshot_status;

      GUIManager gui_manager;

      Interface()
        : context(),
          tcs_online(false),
          err(false),
          subscriber(std::make_unique<Common::PubSub>(context, Common::PubSub::Mode::SUB)),
          is_subscriber_thread_running(false),
          should_subscriber_thread_run(false),
          should_framegrab_run(false),
          is_framegrab_running(false),
          nsave_preserve_frames(0),
          nskip_preserve_frames(0),
          snapshot_status { { "slitd", false }, {"tcsd", false} }
      {
        topic_handlers = {
          { "_snapshot", std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_snapshot(msg); } ) },
          { "tcsd", std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_tcsd(msg); } ) },
          { "slitd", std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_slitd(msg); } ) }
        };
      }

      inline long read_error() { return( this->err.exchange( false ) ? ERROR : NO_ERROR ); };

      inline std::string get_imagename() { return this->imagename; }
      inline std::string get_wcsname()   { return this->wcsname;   }

      inline void set_imagename( std::string name_in ) { this->imagename = ( name_in.empty() ? DEFAULT_IMAGENAME : name_in ); return; }
      inline void set_wcsname( std::string name_in )   { this->wcsname = name_in;   return; }

      Slicecam::FitsInfo fitsinfo;

      Common::Header telemkeys;

      Common::Queue async;                     /// asynchronous message queue

      Camera camera;                           /// 

      FITS_file fits_file;                     /// instantiate a FITS container object

      TcsDaemonClient tcsd;                    /// for communicating with the TCS daemon, defined in ~/Software/tcsd/tcsd_client.h

      Common::DaemonClient acamd { "acamd" };  /// for communicating with acamd

      SkyInfo::FPOffsets fpoffsets;            /// for calling Python fpoffsets, defined in ~/Software/common/skyinfo.h

      // publish/subscribe functions
      //
      long init_pubsub(const std::initializer_list<std::string> &topics={}) {
        return Common::PubSubHandler::init_pubsub(context, *this, topics);
      }
      void start_subscriber_thread() { Common::PubSubHandler::start_subscriber_thread(*this); }
      void stop_subscriber_thread()  { Common::PubSubHandler::stop_subscriber_thread(*this); }

      void handletopic_snapshot( const nlohmann::json &jmessage );
      void handletopic_slitd( const nlohmann::json &jmessage );
      void handletopic_tcsd( const nlohmann::json &jmessage );
      void publish_snapshot();
      void request_snapshot();
      bool wait_for_snapshots();

      long avg_frames( std::string args, std::string &retstring );
      long bin( std::string args, std::string &retstring );
      long test_image();                       ///
      long open( std::string args, std::string &help);    /// wrapper to open all slicecams
      long isopen( std::string which, bool &state, std::string &help );     /// wrapper for slicecams
      bool isopen( std::string which );     /// wrapper for slicecams
      void close();
      long close( std::string args, std::string &retstring );
      long tcs_init( std::string args, std::string &retstring );  /// initialize connection to TCS
      long saveframes( std::string args, std::string &retstring );
      void alert_framegrabbing_stopped(const int &waitms);
      long framegrab( std::string args );                            /// wrapper to control Andor frame grabbing
      long framegrab( std::string args, std::string &retstring );    /// wrapper to control Andor frame grabbing
      long framegrab_fix( std::string args, std::string &retstring );    /// wrapper to control Andor frame grabbing
      long image_quality( std::string args, std::string &retstring );  /// wrapper for Astrometry::image_quality
      long put_on_slit( std::string args, std::string &retstring );  /// put target on slit
      long solve( std::string args, std::string &retstring );  /// wrapper for Astrometry::solve
      long gui_settings_control();          /// get gui settings and push to Guider GUI display
      long gui_settings_control( std::string args, std::string &retstring );  /// set or get and push to Guider GUI display
      long shutdown( std::string args, std::string &retstring );
      long test( std::string args, std::string &retstring );
      long exptime( std::string args, std::string &retstring );
      long fan_mode( std::string args, std::string &retstring );
      long gain( std::string args, std::string &retstring );

      long get_acam_guide_state( bool &is_guiding );

      long collect_header_info( std::unique_ptr<Andor::Interface> &slicecam );

      inline void init_names() { imagename=""; wcsname=""; return; }  // TODO still needed?

      long configure_interface( Config &config );

      static void dothread_fpoffset( Slicecam::Interface &iface );
      void dothread_framegrab( const std::string whattodo, const std::string sourcefile );
      void preserve_framegrab();
      long collect_header_info_threaded();
  };
  /***** Slicecam::Interface **************************************************/

}
/***** Slicecam ***************************************************************/
