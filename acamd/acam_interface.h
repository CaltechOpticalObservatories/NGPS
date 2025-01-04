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
#include "database.h"

#ifdef ANDORSIM
#include "andorsim.h"
#else
#include "andor.h"
#endif

#include "acam_interface_shared.h"

/***** Acam *******************************************************************/
/**
 * @namespace Acam
 * @brief     namespace for acquisition and guide camera
 *
 */
namespace Acam {

  const std::string DAEMON_NAME = "acamd";       ///< when run as a daemon, this is my name

  constexpr std::string_view DEFAULT_IMAGENAME = "/tmp/acam.fits";

  constexpr const char* PYTHON_PATH = "/home/developer/dhale/sandbox/NGPS/Python:/home/developer/dhale/sandbox/NGPS/Python/acam_skyinfo";
  constexpr const char* PYTHON_ASTROMETRY_MODULE = "astrometry";
  constexpr const char* PYTHON_ASTROMETRY_FUNCTION = "astrometry_cwrap";
  constexpr const char* PYTHON_IMAGEQUALITY_MODULE = "image_quality";
  constexpr const char* PYTHON_IMAGEQUALITY_FUNCTION = "image_quality_cwrap";

  constexpr double PI = 3.14159265358979323846;

  enum FocusThreadStates {
    FOCUS_MONITOR_STOPPED,
    FOCUS_MONITOR_STOP_REQ,
    FOCUS_MONITOR_START_REQ,
    FOCUS_MONITOR_RUNNING
  };

  enum TargetAcquisitionModes {
    TARGET_NOP = 0,
    TARGET_ACQUIRE,
    TARGET_ACQUIRE_HERE,
    TARGET_GUIDE,
    TARGET_MODE_COUNT
  };

  const std::string TargetAcquisitionModeString[] = {
    "standby",
    "acquiring",
    "acquiring",
    "guiding"
  };

  class Interface;  // forward declaration since it's used in the Target class

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
      int simsize;      /// for the sky simulator
      std::map<at_32, at_32> handlemap;

    public:
      Camera() : image_data( nullptr ), simsize(1024) { };

      FITS_file fits_file;        /// instantiate a FITS container object
      FitsInfo  fitsinfo;

      Andor::Interface andor;     ///< create an Andor::Interface object for interfacing with the camera

      Andor::DefaultValues default_config;  ///< holds defaults for the camera

      inline void copy_info() { fits_file.copy_info( fitsinfo ); }
      inline void set_simsize( int val )     { if ( val > 0 ) this->simsize = val;  else throw std::out_of_range("simsize must be greater than 0");  }

      long emulator( std::string args, std::string &retstring );
      long open( int sn );
      long close();
      long start_acquisition();
      long get_frame();
      long write_frame( std::string source_file, std::string &outfile, const bool _tcs_online );
      long get_status();
      long bin( const int hbin, const int vbin );
      long imflip( std::string args, std::string &retstring );
      long imrot( std::string args, std::string &retstring );
      long gain( std::string args, std::string &retstring );
      long gain( int value );
      int gain();
      long set_exptime( float &fval );
      long set_exptime( float &&fval );
      long set_fan( int mode );
      long speed( std::string args, std::string &retstring );
      long temperature( std::string args, std::string &retstring );
  };
  /***** Acam::Camera *********************************************************/


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
      double ra, dec, pa, rmsarcsec;
      long matches;
      double seeing, seeing_std, seeing_zen, extinction, extinction_std, background, background_std;
      bool python_initialized;

    public:
      Astrometry()
        : ra(NAN), dec(NAN), pa(NAN), rmsarcsec(NAN),
          matches(-1), seeing(NAN), seeing_std(NAN), seeing_zen(NAN),
          extinction(NAN), extinction_std(NAN),
          background(NAN), background_std(NAN),
          python_initialized(false), isacquire(true), pAstrometryModule(nullptr), pQualityModule(nullptr) { }

      long initialize_python();                          /// initializes the Python module

      bool isacquire;
      inline bool is_initialized() { return this->python_initialized; };
      std::vector<std::string> solver_args;              /// contains list of optional solver args, "key1=val key2=val ... keyN=val"

      CPython::CPyInstance py_instance { PYTHON_PATH };  /// initialize the Python interpreter

      PyObject* pAstrometryModule;                       /// astrometry
      PyObject* pQualityModule;                          /// image quality

      inline double get_seeing()         const { return this->seeing; }
      inline double get_seeing_std()     const { return this->seeing_std; }
      inline double get_seeing_zen()     const { return this->seeing_zen; }
      inline double get_extinction()     const { return this->extinction; }
      inline double get_extinction_std() const { return this->extinction_std; }
      inline double get_background()     const { return this->background; }
      inline double get_background_std() const { return this->background_std; }

      inline void get_solution( std::string &_result, double &_ra, double &_dec, double &_angle,
                                long &_matches, double &_rmsarcsec ) const {
        _result    = this->result;
        _ra        = this->ra;
        _dec       = this->dec;
        _angle     = this->pa;
        _matches   = this->matches;
        _rmsarcsec = this->rmsarcsec;
      }

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
  };
  /***** Acam::Astrometry *****************************************************/


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
      std::string push_message;  ///<! name of script to push a message to GUI

    public:
      GuideManager() : update(false), exptime(NAN), gain(-1), filter("undef"), navg(NAN) { }

      // These are the GUIDER GUI settings
      //
      float exptime;
      int gain;
      std::string filter;  // Python needs an arg so filter can't be empty-initialize in constructor
      float navg;
      std::string status;

      // sets the private variable push_settings, call on config
      inline void set_push_settings( std::string sh ) { this->push_settings=sh; }

      // sets the private variable push_image, call on config
      inline void set_push_image( std::string sh ) { this->push_image=sh; }

      // sets the private variable push_image, call on config
      inline void set_push_message( std::string sh ) { this->push_message=sh; }

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
       * @return     string in form of <exptime> <gain> <filter> <navg>
       */
      std::string get_message_string() {
        std::stringstream message;
        if ( this->exptime < 0 ) message << "ERR"; else { message << std::fixed << std::setprecision(3) << this->exptime; }
        message << " ";
        if ( this->gain < 1 ) message << "ERR"; else { message << std::fixed << std::setprecision(3) << this->gain; }
        message << " ";
        message << ( this->filter.empty() ? "undef" : this->filter );  // Python will need an arg so filter can't be empty
        message << " ";
        if ( std::isnan( this->navg ) ) message << "ERR"; else { message << std::fixed << std::setprecision(2) << this->navg; };
        message << " " << status;
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

      /**
       * @brief      calls the push_message script with the supplied message string
       * @param[in]  message  message to send
       */
      void push_guider_message( std::string_view message ) {
        std::string function = "Acam::GuideManager::push_guider_message";
        std::stringstream cmd;
        cmd << push_message << " "
            << camera_name << " "
            << message;

        if ( std::system( cmd.str().c_str() ) && errno!=ECHILD ) {
          logwrite( function, "ERROR pushing message to GUI" );
        }

        return;
      }
  };
  /***** Acam::GuideManager ***************************************************/


  /***** Acam::Target *********************************************************/
  /**
   * @class   Target
   * @brief   defines functions and settings for target and target acquisition
   * @details This class contains info about the target (coords) and related
   *          functions, such as the target acquisition sequence. It declares
   *          the Acam::Interface class as a friend, so that it can directly
   *          call Interface functions.
   *
   */
  class Target {

    friend class Acam::Interface;          ///< Make Interface a friend of Target

    private:
      Acam::Interface* iface;              ///< Pointer to the Interface instance

      double offset_threshold;  ///< successful acquisition when computed offset below config ACQUIRE_OFFSET_THRESHOLD
      double timeout;           ///< target acquisition timeout (sec) from config ACAM_ACQUIRE_TIMEOUT
      int max_attempts;         ///< max number of acquisition loop attempts
      int min_repeat;           ///< minimum sequential successful acquires from config ACQUIRE_MIN_REPEAT

      int nacquired;
      int attempts;
      int sequential_failures;

      std::atomic<bool> is_acquired;       ///< set if target acquired successfully
      std::atomic<bool> stop_acquisition;  ///< set if the acquisition sequence should stop

      double tcs_max_offset;

      double offset_cal_offset, offset_cal_raoff, offset_cal_decoff;

      std::chrono::time_point<std::chrono::steady_clock,
                              std::chrono::duration<double>> timeout_time, time_last_offset;

      std::vector<double> ra_off_list, dec_off_list;  ///< lists of offsets for median filtering
      std::chrono::seconds::rep tcs_offset_period;    ///< period at which to send offsets while guiding

      struct coords_t {
        double ra;
        double dec;
        double angle;
      };                                   ///< a structure to hold target coordinates, always decimal degrees

      coords_t coords_slit;                ///< slit coordinates

      double tcs_casangle;                 ///< the cass angle from the TCS, must be supplied

      std::string name;                    ///< target name
      std::string pointmode;

    public:

      inline std::chrono::seconds::rep get_tcs_offset_period() { return this->tcs_offset_period; }
      inline void set_tcs_offset_period(std::chrono::seconds::rep val) { this->tcs_offset_period=val; }

      inline double get_timeout() { return this->timeout; }

      inline long set_timeout( const double _timeout ) {
        if ( std::isnan( _timeout ) || _timeout <= 0 ) return ERROR;
        else {
          this->timeout = _timeout;
          return NO_ERROR;
        }
      }

      inline long set_offset_threshold( const double _thresh ) {
        if ( std::isnan( _thresh ) || _thresh <= 0 ) return ERROR;
        else {
          this->offset_threshold = _thresh;
          return NO_ERROR;
        }
      }

      inline long set_tcs_max_offset( const double _offset ) {
        if ( std::isnan( _offset ) || _offset <= 0 ) return ERROR;
        else {
          this->tcs_max_offset = _offset;
          return NO_ERROR;
        }
      }

      inline double get_tcs_max_offset() { return this->tcs_max_offset; }

      inline void set_max_attempts( int _max ) { this->max_attempts = _max; }
      inline void set_min_repeat( int _repeat ) { this->min_repeat = _repeat; }

      std::vector<std::string> ext_solver_args;   ///< externally-set solver args (probably only for testing)

      std::atomic<Acam::TargetAcquisitionModes> acquire_mode;  ///< enum list of possible acquisition modes

      /* @brief  return a human-friendly string of the target mode
       */
      inline std::string acquire_mode_string() const {
        if ( acquire_mode >= 0 && acquire_mode < Acam::TARGET_MODE_COUNT ) {
          return TargetAcquisitionModeString[ acquire_mode ];
        }
        else return "unknown";
      }

      inline void set_interface_instance( Acam::Interface* iface_in ) { iface = iface_in; }

      long acquire( Acam::TargetAcquisitionModes requested_mode );
      long do_acquire();
      bool median_filter( double &ra_off, double &dec_off );

      inline void save_casangle( const double _angle ) { this->tcs_casangle = _angle; }

      inline void set_coords( const double _ra, const double _dec, const double _angle, const std::string _name ) {
        this->coords_slit.ra    = _ra;
        this->coords_slit.dec   = _dec;
        this->coords_slit.angle = _angle;
        this->name              = _name;
      }

      inline void get_coords( double &_ra, double &_dec, double &_angle ) {
        _ra    = this->coords_slit.ra;
        _dec   = this->coords_slit.dec;
        _angle = this->coords_slit.angle;;
      }

      inline const std::string get_name() { return this->name; };

      inline const std::string get_pointmode() { return this->pointmode; };

      void set_pointmode( const std::string pm ) {
        if ( pm != Acam::POINTMODE_SLIT && pm != Acam::POINTMODE_ACAM ) {
          std::stringstream message;
          message << "ERROR invalid pointmode \"" << pm << "\". expected { "
                  << POINTMODE_SLIT << " " << POINTMODE_ACAM << " }";
          logwrite( "Acam::Target::set_pointmode", message.str() );
        }
        this->pointmode = pm;
      }

      double dRA, dDEC;  /// offsets from put_on_slit will be applied to goal while guiding

      struct {
        double ra;
        double dec;
        double angle;
      } acam_goal;

      double putonslit_offset, last_putonslit_offset;

      Target() : iface(nullptr), timeout(10), max_attempts(-1), min_repeat(1),
                 is_acquired(false),
                 stop_acquisition(false),
                 offset_cal_offset(0),
                 timeout_time(std::chrono::steady_clock::now()),
                 time_last_offset(std::chrono::steady_clock::now()),
                 tcs_offset_period(1),
                 pointmode(Acam::POINTMODE_SLIT),
                 acquire_mode(Acam::TARGET_NOP),
                 dRA(0), dDEC(0),
                 putonslit_offset(0), last_putonslit_offset(0) { }
  };
  /***** Acam::Target *********************************************************/


  /***** Acam::Interface ******************************************************/
  /**
   * @class   Interface
   * @brief   interface class for acam
   * @details This class defines the interface for the acam system and contains
   *          the functions used to communicate with it. The Target class is
   *          my friend so I share an instance of my *this pointer so that he
   *          can call my functions.
   *
   */
  class Interface {
    private:
      zmqpp::context context;
      std::mutex framegrab_mutex;
      std::atomic<Acam::FocusThreadStates> monitor_focus_state;
      std::atomic<bool> tcs_online;         ///< set if TCS is online / connected
      std::string imagename;
      std::string wcsname;
      std::chrono::steady_clock::time_point wcsfix_time;
      std::chrono::steady_clock::time_point framegrab_time;
      std::mutex framegrab_mtx;
      std::condition_variable cv;

    public:

      std::string motion_host;
      int motion_port;

      std::atomic<bool> should_framegrab_run;  ///< set if framegrab loop should run
      std::atomic<bool> is_framegrab_running;  ///< set if framegrab loop is running
      std::atomic<bool> is_shutting_down;      ///< set during shutdown

      std::atomic<int> nskip_before_acquire;   ///< number of frame grabs to skip before using for target acquisition

      /** these are set by Interface::saveframes()
       */
      std::atomic<int> nsave_preserve_frames;  ///< number of frames to preserve (normally overwritten)
      std::atomic<int> nskip_preserve_frames;  ///< number of frames to skip before saving nsave... frames


      std::vector<std::string> db_info;        ///< info for constructing telemetry Database object

      std::map<std::string, int> telemetry_providers;  ///< map of port[daemon_name] for external telemetry providers

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
      } telem;

      std::mutex snapshot_mtx;
      std::unordered_map<std::string, bool> snapshot_status;

      nlohmann::json tcs_jmessage;             ///< JSON message containing TCS telemetry

      Interface()
        : context(),
          monitor_focus_state(Acam::FOCUS_MONITOR_STOPPED),
          tcs_online(false),
          motion_port(-1),
          should_framegrab_run(false),
          is_framegrab_running(false),
          is_shutting_down(false),
          nskip_before_acquire(0),
          nsave_preserve_frames(0),
          nskip_preserve_frames(0),
          snapshot_status {
            {"tcsd",       false},
            {"slitd",      false}
          },
          subscriber(std::make_unique<Common::PubSub>(context, Common::PubSub::Mode::SUB)),
          is_subscriber_thread_running(false),
          should_subscriber_thread_run(false)
      {
            target.set_interface_instance( this ); ///< Set the Interface instance in Target
            topic_handlers = {
              { "_snapshot", std::function<void(const nlohmann::json&)>(
                         [this](const nlohmann::json &msg) { handletopic_snapshot(msg); } ) },
              { "tcsd", std::function<void(const nlohmann::json&)>(
                         [this](const nlohmann::json &msg) { handletopic_tcsd(msg); } ) },
              { "targetinfo", std::function<void(const nlohmann::json&)>(
                         [this](const nlohmann::json &msg) { handletopic_targetinfo(msg); } ) },
              { "slitd", std::function<void(const nlohmann::json&)>(
                         [this](const nlohmann::json &msg) { handletopic_slitd(msg); } ) }
            };
      }

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

      inline bool is_target_acquired()   { return this->target.is_acquired; }
      inline std::string get_imagename() { return this->imagename; }
      inline std::string get_wcsname()   { return this->wcsname;   }

      inline void set_imagename( std::string name_in ) { this->imagename = ( name_in.empty() ? DEFAULT_IMAGENAME : name_in ); return; }
      inline void set_wcsname( std::string name_in )   { this->wcsname = name_in;   return; }

      GuideManager guide_manager;

      Database::Database database;

      Acam::FitsInfo fitsinfo;

      Common::Header telemkeys;                ///< subscribed telemetry goes here

      Target target;                           /// for target acquisition

      Common::Queue async;                     /// asynchronous message queue

      Camera camera;                           /// 

      MotionInterface motion;                  /// this object provides the interface to the filter and dust cover

      Astrometry astrometry;                   /// provides the interface to the Python astrometry packages

      FITS_file fits_file;                     /// instantiate a FITS container object

      TcsDaemonClient tcsd;                    /// for communicating with the TCS daemon, defined in ~/Software/tcsd/tcsd_client.h

      SkyInfo::FPOffsets fpoffsets;            /// for calling Python fpoffsets, defined in ~/Software/common/skyinfo.h

      // publish/subscribe functions
      //
      long init_pubsub(const std::initializer_list<std::string> &topics={}) {
        return Common::PubSubHandler::init_pubsub(context, *this, topics);
      }
      void start_subscriber_thread() { Common::PubSubHandler::start_subscriber_thread(*this); }
      void stop_subscriber_thread()  { Common::PubSubHandler::stop_subscriber_thread(*this); }

      void handletopic_snapshot( const nlohmann::json &jmessage );
      void handletopic_tcsd( const nlohmann::json &jmessage );
      void handletopic_targetinfo( const nlohmann::json &jmessage );
      void handletopic_slitd( const nlohmann::json &jmessage );

      long bin( std::string args, std::string &retstring );
      void publish_snapshot();
      void request_snapshot();
      bool wait_for_snapshots();
      long handle_json_message( std::string message_in );
      long initialize_python_objects();        /// provides interface to initialize all Python modules for objects in this class
      long test_image();                       ///
      long open( std::string args, std::string &help);    /// wrapper to open all acam-related hardware components
      long isopen( std::string component, bool &state, std::string &help );     /// wrapper for acam-related hardware components
      bool isopen( std::string component );     /// wrapper for acam-related hardware components
      void close();
      long close( std::string component, std::string &help );      ///
      long tcs_init( std::string args, std::string &retstring );  /// initialize connection to TCS
      long framegrab( std::string args, std::string &retstring );    /// wrapper to control Andor frame grabbing
      long framegrab_fix( std::string args, std::string &retstring );    /// wrapper to control Andor frame grabbing
      long saveframes( std::string args, std::string &retstring );    /// set/get number of frame grabgs to save during acquisition
      long skipframes( std::string args, std::string &retstring );    /// set/get number of frame grabgs to skip before target acquisition
      long image_quality( std::string args, std::string &retstring );  /// wrapper for Astrometry::image_quality
      long solve( std::string args, std::string &retstring );  /// wrapper for Astrometry::solve
      long guider_settings_control();          /// get guider settings and push to Guider GUI display
      long guider_settings_control( std::string args, std::string &retstring );  /// set or get and push to Guider GUI display
      long avg_frames( std::string args, std::string &retstring );
      long offset_period( std::string args, std::string &retstring );
      long acquire( std::string args, std::string &retstring );
      long target_coords( std::string args, std::string &retstring );  /// set or get target coords for acquire
      long offset_cal( const std::string args, std::string &retstring );
      long offset_goal( const std::string args, std::string &retstring );
      long put_on_slit( const std::string args, std::string &retstring );
      void preserve_framegrab();
      long shutdown( std::string args, std::string &retstring );
      long test( std::string args, std::string &retstring );
      long exptime( const std::string args, std::string &retstring );
      long fan_mode( std::string args, std::string &retstring );

      long collect_header_info();

      inline void init_names() { imagename=""; wcsname=""; return; }  // TODO still needed?

      long configure_interface( Config &config );

      static void dothread_fpoffset( Acam::Interface &iface );
      static void dothread_framegrab( Acam::Interface &iface, const std::string whattodo, std::string sourcefile );
      static void dothread_set_filter( Acam::Interface &iface, std::string filter_req );
      static void dothread_set_focus( Acam::Interface &iface, double focus_req );
      static void dothread_monitor_focus( Acam::Interface &iface );
  };
  /***** Acam::Interface ******************************************************/

}
/***** Acam *******************************************************************/
