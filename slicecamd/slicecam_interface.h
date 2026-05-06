/** ---------------------------------------------------------------------------
 * @file    slicecam_interface.h
 * @brief   slicecam interface include
 * @details defines the classes used by the slicecam hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <optional>
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
#include "acamd_commands.h"
#include "tcsd_client.h"
#include "skyinfo.h"
#include "slicecam_camera.h"
#include "slicecam_math.h"
#include "message_keys.h"

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

#include "guimanager.h"

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

  /***** Slicecam::FineAcqState ***********************************************/
  /**
   * @brief  Persistent state for the fine-acquisition per-frame state machine
   *
   * @details
   *   which       - which camera to use ("L" or "R")
   *   aimpoint    - desired star location on the chip, FITS 1-based pixels.
   *                 This is the pixel analogue of CF's --goal-x / --goal-y.
   *                 The star is driven toward this pixel, not toward an ra/dec.
   *   bg_region   - background estimation ROI, 1-based inclusive.
   *                 Matches the --bg-x1/x2/y1/y2 defaults in slicecamd.cfg.
   *   dra_samp    - accumulated dRA*cos(dec) samples in degrees
   *   ddec_samp   - accumulated dDEC samples in degrees
   *   max_samples - samples to gather before evaluating a move
   *   goal_arcsec - convergence threshold; loop stops when median offset
   *                 magnitude falls below this value
   *   gain        - fraction of the median offset commanded each cycle (0..1)
   *   skip_frames - counts down frames to discard while telescope settles
   *                 after a commanded move
   */
  struct FineAcqState {
    std::string which;
    Point       aimpoint { NAN, NAN };  ///< 1-based pixel aim point; NAN until configured
    Rect        bg_region;          ///< background ROI (1-based)
    std::vector<double> dra_samp;   ///< dRA*cos(dec) samples, degrees
    std::vector<double> ddec_samp;  ///< dDEC samples, degrees
    int    max_samples  = 10;       ///< samples before evaluating a move
    double goal_arcsec  = 0.3;      ///< convergence threshold, arcsec
    double gain         = 0.7;      ///< gain applied to commanded offset
    int    skip_frames  = 0;        ///< frames to skip after a telescope move

    void reset() { dra_samp.clear(); ddec_samp.clear(); skip_frames = 0; }
    bool is_valid() const noexcept {
      return !which.empty() && aimpoint.is_valid() && bg_region.is_valid();
    }
  };
  /***** Slicecam::FineAcqState ***********************************************/


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

      std::mutex acam_mtx;                 ///< guards waiters on acam_cv
      std::condition_variable acam_cv;     ///< notified when cached ACAM state updates

      FineAcqState fineacquire_state;

      std::string default_which;                    ///< configured default camera for fineacquire
      Point       default_aimpoint { NAN, NAN };    ///< configured default aimpoint for fineacquire

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
      std::atomic<bool> is_fineacquire_running;  ///< set if fine target acquisition is running
      std::atomic<bool> is_fineacquire_locked;   ///< set when fine acquire target acquired
      std::atomic<bool> is_acam_guiding;         ///< is acam guiding?

      std::atomic<int64_t> last_acam_pubtime{0};   ///< pubtime (us) of latest received acamd status

      /// Max acceptable age (us) for cached ACAM status used by fineacquire.
      static constexpr int64_t ACAM_STATUS_MAX_AGE_US = 10'000'000;

      /// Max time (us) fineacquire() will wait for a fresh, guiding ACAM status before failing.
      static constexpr int64_t ACAM_WAIT_TIMEOUT_US = 2'000'000;

      bool is_acam_status_fresh() const;

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

      struct {
        bool is_fineacquire_running=false;
        bool is_fineacquire_locked=false;
      } last_status;

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
          is_fineacquire_running(false),
          is_fineacquire_locked(false),
          is_acam_guiding(false),
          nsave_preserve_frames(0),
          nskip_preserve_frames(0),
          snapshot_status { { Topic::SLITD, false },
	                    { Topic::TCSD, false },
	                    { Topic::ACAMD, false } }
      {
        topic_handlers = {
          { Topic::SNAPSHOT, std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_snapshot(msg); } ) },
          { Topic::ACAMD, std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_acamd(msg); } ) },
          { Topic::TCSD, std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_tcsd(msg); } ) },
          { Topic::SLITD, std::function<void(const nlohmann::json&)>(
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
      void handletopic_acamd( const nlohmann::json &jmessage );
      void handletopic_slitd( const nlohmann::json &jmessage );
      void handletopic_tcsd( const nlohmann::json &jmessage );
      void publish_status(bool force=false);
      void publish_snapshot();
      void request_snapshot();
      bool wait_for_snapshots();

      long fineacquire(std::string args, std::string &retstring);
      void do_fineacquire();

      long avg_frames( std::string args, std::string &retstring );
      long bin( std::string args, std::string &retstring );
      long test_image();                       ///
      long open( std::string args, std::string &help);    /// wrapper to open all slicecams
      long isopen( std::string which, bool &state, std::string &help );     /// wrapper for slicecams
      bool isopen( std::string which );     /// wrapper for slicecams
      void close();
      long close( std::string args, std::string &retstring );
      long tcs_init( std::string args, std::string &retstring );  /// initialize connection to TCS
      long acamd_init();
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
      long shutter(const std::string args, std::string &retstring);
      long test( std::string args, std::string &retstring );
      long exptime( std::string args, std::string &retstring );
      long fan_mode( std::string args, std::string &retstring );
      long gain( std::string args, std::string &retstring );

      long offset_acam_goal(const std::pair<double, double> &offsets, std::optional<bool> fineacquire=std::nullopt);

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
