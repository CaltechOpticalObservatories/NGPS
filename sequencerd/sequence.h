/**
 * @file     sequence.h
 * @brief    header file for the Sequence class which contains the sequence functions
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 * Ths Sequencer::Sequence class contains functions which perform the 
 * sequence operations which are called by the sequencer daemon.
 *
 */
#pragma once

#include <memory>
#include <thread>
#include <future>
#include <vector>
#include <chrono>
#include <atomic>
#include <map>
#include <cmath>
#include <mysqlx/xdevapi.h>
#include <json.hpp>

#include "sequencer_interface.h"  // this defines the classes used to interface with various subsystems

#include "acamd_commands.h"
#include "slicecamd_commands.h"
#include "calibd_commands.h"
#include "camerad_commands.h"
#include "flexured_commands.h"
#include "focusd_commands.h"
#include "powerd_commands.h"
#include "slitd_commands.h"
#include "tcsd_commands.h"
#include "sequencerd_commands.h"

#include "tcs_constants.h"
#include "acam_interface_shared.h"

/***** Sequencer **************************************************************/
/**
 * @namespace Sequencer
 * @brief     namespace for the observation sequencer
 *
 */
namespace Sequencer {

  /**
   * @enum  DaemonBits
   * @brief assigns each subsystem a bit to indicate its state
   */
  enum DaemonBits : size_t {
    DAEMON_ACAM=0,
    DAEMON_CALIB,
    DAEMON_CAMERA,
    DAEMON_FLEXURE,
    DAEMON_FOCUS,
    DAEMON_POWER,
    DAEMON_SLICECAM,
    DAEMON_SLIT,
    DAEMON_TCS,
    NUM_DAEMONS
  };

  const std::map<size_t, std::string> daemon_names = {
    {DAEMON_ACAM,      "acam"},
    {DAEMON_CALIB,     "calib"},
    {DAEMON_CAMERA,    "camera"},
    {DAEMON_FLEXURE,   "flexure"},
    {DAEMON_FOCUS,     "focus"},
    {DAEMON_POWER,     "power"},
    {DAEMON_SLICECAM,  "slicecam"},
    {DAEMON_SLIT,      "slit"},
    {DAEMON_TCS,       "tcs"}
  };

  /**
   * @enum  SequenceStateBits
   * @brief assigns each subsystem a bit to indicate its state
   */
  enum SequenceStateBits : size_t {
    SEQ_OFFLINE=0,           ///< set when when offline
    SEQ_ABORTREQ,            ///< set when an abort is requested
    SEQ_STOPREQ,             ///< set when a stop is requested
    SEQ_NOTREADY,            ///< set when sequencer is not ready
    SEQ_READY,               ///< set when sequencer is ready to be started
    SEQ_RUNNING,             ///< set when sequencer is running
    SEQ_SHUTTING,            ///< set when sequencer is shutting down
    SEQ_PAUSE,               ///< set when sequencer is paused
    SEQ_STARTING,            ///< set when sequencer is starting up
    SEQ_WAIT_ACAM,           ///< set when waiting for acam
    SEQ_WAIT_SLICECAM,       ///< set when waiting for slicecam
    SEQ_WAIT_ACQUIRE,        ///< set when waiting for acquire
    SEQ_WAIT_CALIB,          ///< set when waiting for calib
    SEQ_WAIT_CAMERA,         ///< set when waiting for camera
    SEQ_WAIT_EXPOSE,         ///< set when waiting for camera exposure
    SEQ_GUIDE,               ///< set/clear to enable/disable guide thread
    SEQ_WAIT_READOUT,        ///< set when waiting for camera readout
    SEQ_WAIT_FILTER,         ///< set when waiting for filter
    SEQ_WAIT_FLEXURE,        ///< set when waiting for flexure
    SEQ_WAIT_FOCUS,          ///< set when waiting for focus
    SEQ_WAIT_POWER,          ///< set when waiting for power
    SEQ_WAIT_SLIT,           ///< set when waiting for slit
    SEQ_WAIT_TCS,            ///< set when waiting for tcs
    SEQ_WAIT_TCSOP,          ///< set when waiting specifically for tcs operator
    SEQ_WAIT_USER,           ///< set when waiting specifically for user input
    SEQ_WAIT_SETTLE,         ///< set when waiting specifically for tcs settle
    SEQ_WAIT_SLEW,           ///< set when waiting specifically for tcs slew
    NUM_SEQ_STATES
  };

  const std::map<size_t, std::string> seq_state_names = {
    {SEQ_OFFLINE,       "OFFLINE"},
    {SEQ_ABORTREQ,      "ABORTREQ"},
    {SEQ_STOPREQ,       "STOPREQ"},
    {SEQ_NOTREADY,      "NOTREADY"},
    {SEQ_READY,         "READY"},
    {SEQ_RUNNING,       "RUNNING"},
    {SEQ_SHUTTING,      "SHUTTING"},
    {SEQ_PAUSE,         "PAUSE"},
    {SEQ_STARTING,      "STARTING"},
    {SEQ_WAIT_ACAM,     "ACAM"},
    {SEQ_WAIT_SLICECAM, "SLICECAM"},
    {SEQ_WAIT_ACQUIRE,  "ACQUIRE"},
    {SEQ_WAIT_CALIB,    "CALIB"},
    {SEQ_WAIT_CAMERA,   "CAMERA"},
    {SEQ_WAIT_EXPOSE,   "EXPOSE"},
    {SEQ_GUIDE,         "GUIDE"},
    {SEQ_WAIT_READOUT,  "READOUT"},
    {SEQ_WAIT_FILTER,   "FILTER"},
    {SEQ_WAIT_FLEXURE,  "FLEXURE"},
    {SEQ_WAIT_FOCUS,    "FOCUS"},
    {SEQ_WAIT_POWER,    "POWER"},
    {SEQ_WAIT_SLIT,     "SLIT"},
    {SEQ_WAIT_TCS,      "TCS"},
    {SEQ_WAIT_TCSOP,    "TCSOP"},
    {SEQ_WAIT_USER,     "USER"},
    {SEQ_WAIT_SETTLE,   "SETTLE"},
    {SEQ_WAIT_SLEW,     "SLEW"}
  };

  /**
   * @enum     ThreadStatusBits
   * @brief    assigns each thread a bit in a threadstate word
   * @details  bit is set while thread is running
   */
  enum ThreadStatusBits : size_t {
    THR_SEQUENCER_ASYNC_LISTENER=0,
    THR_TRIGGER_EXPOSURE,
    THR_REPEAT_EXPOSURE,
    THR_STOP_EXPOSURE,
    THR_ABORT_PROCESS,
    THR_SEQUENCE_START,
    THR_MONITOR_READY_STATE,
    THR_CALIB_SET,
    THR_CAMERA_SET,
    THR_SLIT_SET,
    THR_MOVE_TO_TARGET,
    THR_NOTIFY_TCS,
    THR_FOCUS_SET,
    THR_FLEXURE_SET,
    THR_CALIBRATOR_SET,
    THR_ACAM_INIT,
    THR_ANDOR_INIT,
    THR_SLICECAM_INIT,
    THR_CALIB_INIT,
    THR_TCS_INIT,
    THR_SLIT_INIT,
    THR_CAMERA_INIT,
    THR_FLEXURE_INIT,
    THR_FOCUS_INIT,
    THR_POWER_INIT,
    THR_ACAM_SHUTDOWN,
    THR_ANDOR_SHUTDOWN,
    THR_SLICECAM_SHUTDOWN,
    THR_CALIB_SHUTDOWN,
    THR_CAMERA_SHUTDOWN,
    THR_FLEXURE_SHUTDOWN,
    THR_FOCUS_SHUTDOWN,
    THR_SLIT_SHUTDOWN,
    THR_TCS_SHUTDOWN,
    THR_POWER_SHUTDOWN,
    THR_MODIFY_EXPTIME,
    THR_ACQUISITION,
    THR_GUIDING,
    THR_STARTUP,
    THR_SHUTDOWN,
    NUM_THREAD_STATES
  };

  const std::map<size_t, std::string> thread_names = {
    {THR_SEQUENCER_ASYNC_LISTENER, "async_listener"},
    {THR_TRIGGER_EXPOSURE,         "trigger_exposure"},
    {THR_REPEAT_EXPOSURE,          "repeat_exposure"},
    {THR_STOP_EXPOSURE,            "stop_exposure"},
    {THR_ABORT_PROCESS,            "abort_process"},
    {THR_SEQUENCE_START,           "sequence_start"},
    {THR_MONITOR_READY_STATE,      "monitor_ready_state"},
    {THR_CALIB_SET,                "calib_set"},
    {THR_CAMERA_SET,               "camera_set"},
    {THR_SLIT_SET,                 "slit_set"},
    {THR_MOVE_TO_TARGET,           "move_to_target"},
    {THR_NOTIFY_TCS,               "notify_tcs"},
    {THR_FOCUS_SET,                "focus_set"},
    {THR_FLEXURE_SET,              "flexure_set"},
    {THR_CALIBRATOR_SET,           "calibrator_set"},
    {THR_ACAM_INIT,                "acam_init"},
    {THR_ANDOR_INIT,               "andor_init"},
    {THR_SLICECAM_INIT,            "slicecam_init"},
    {THR_CALIB_INIT,               "calib_init"},
    {THR_TCS_INIT,                 "tcs_init"},
    {THR_SLIT_INIT,                "slit_init"},
    {THR_CAMERA_INIT,              "camera_init"},
    {THR_FLEXURE_INIT,             "flexure_init"},
    {THR_FOCUS_INIT,               "focus_init"},
    {THR_POWER_INIT,               "power_init"},
    {THR_ACAM_SHUTDOWN,            "acam_shutdown"},
    {THR_ANDOR_SHUTDOWN,           "andor_shutdown"},
    {THR_SLICECAM_SHUTDOWN,        "slicecam_shutdown"},
    {THR_CALIB_SHUTDOWN,           "calib_shutdown"},
    {THR_CAMERA_SHUTDOWN,          "camera_shutdown"},
    {THR_FLEXURE_SHUTDOWN,         "flexure_shutdown"},
    {THR_FOCUS_SHUTDOWN,           "focus_shutdown"},
    {THR_SLIT_SHUTDOWN,            "slit_shutdown"},
    {THR_TCS_SHUTDOWN,             "tcs_shutdown"},
    {THR_POWER_SHUTDOWN,           "power_shutdown"},
    {THR_MODIFY_EXPTIME,           "modify_exptime"},
    {THR_ACQUISITION,              "acquisition"},
    {THR_GUIDING,                  "guiding"},
    {THR_STARTUP,                  "startup"},
    {THR_SHUTDOWN,                 "shutdown"}
  };

  /***** Sequencer::Sequence **************************************************/
  /**
   * @class Sequence
   * @brief the Sequence class defines the "sequences", i.e. functions of the sequencer
   */
  class Sequence {
    private:
      bool ready_to_start;                       ///< set on nightly startup success, used to return seqstate to READY after an abort
      std::atomic<bool> notify_tcs_next_target;  ///< notify TCS of next target when remaining time within TCS_PREAUTH_TIME
      std::atomic<bool> arm_readout_flag;        ///< 
      std::atomic<bool> cancel_flag{false};
      std::atomic<bool> is_ontarget{false};      ///< remotely set by the TCS operator to indicate that the target is ready
      std::atomic<bool> is_usercontinue{false};  ///< remotely set by the user to continue
    public:
      Sequence() :
          ready_to_start(false),
          notify_tcs_next_target(false),
          arm_readout_flag(false),
          acquisition_timeout(0),
          acquisition_max_retrys(-1),
          tcs_offsetrate_ra(45),
          tcs_offsetrate_dec(45),
          tcs_settle_timeout(10),
          tcs_settle_stable(1),
          tcs_domeazi_ready(1),
          tcs_preauth_time(0),
          do_once(false),
          tcs_which("real"),
          tcs_name("offline") {
            seq_state_manager.set_callback([this](const std::bitset<NUM_SEQ_STATES>& states) { improved_broadcast_seqstate(); });
            thread_state_manager.set_callback([this](const std::bitset<NUM_THREAD_STATES>& states) { broadcast_threadstate(); });
            daemon_manager.set_callback([this](const std::bitset<NUM_DAEMONS>& states) { broadcast_daemonstate(); });
            seq_state.set( Sequencer::SEQ_OFFLINE );
            req_state.set( Sequencer::SEQ_OFFLINE );
            seq_state_manager.set(Sequencer::SEQ_NOTREADY);
          }

      ~Sequence() { };

      inline void ontarget() {
        this->cancel_flag.store(false);
        this->is_ontarget.store(true);
        this->cv.notify_all();
      }
      inline void usercontinue() {
        this->cancel_flag.store(false);
        this->is_usercontinue.store(true);
        this->cv.notify_all();
      }
      inline void reset_cancel_flag() { this->cancel_flag.store(false); }

      std::map<std::string, int> telemetry_providers;  ///< map of port[daemon_name] for external telemetry providers

      double acquisition_timeout; ///< timeout for target acquisition (in sec) set by configuration parameter ACAM_ACQUIRE_TIMEOUT
      int acquisition_max_retrys; ///< max number of acquisition loop attempts
      double tcs_offsetrate_ra;   ///< TCS offset rate RA ("MRATE") in arcsec per second
      double tcs_offsetrate_dec;  ///< TCS offset rate DEC ("MRATE") in arcsec per second
      double tcs_settle_timeout;  ///< timeout for telescope to settle (in sec) set by configuration parameter TCS_SETTLE_TIMEOUT
      double tcs_settle_stable;   ///< time that TCS must report TRACKING before it is really tracking
      double tcs_domeazi_ready;   ///< max degrees azimuth that dome and telescope can differ before ready to observe
      double tcs_preauth_time;    ///< seconds before end of exposure to notify TCS of next target's coords (0 to disable)

///   std::mutex              tcs_ontarget_mtx;
///   std::condition_variable tcs_ontarget_cv;

      std::mutex wait_mtx;
      std::condition_variable cv;
      std::mutex cv_mutex;
      std::mutex monitor_mtx;

      std::map<int, std::string> sequence_states;
      std::vector<int> sequence_state_bits;

      std::atomic<bool> do_once;            ///< set if "do one" selected, clear if "do all" selected

      std::mutex seqstate_mtx;
      std::condition_variable seqstate_cv;

      StateManager<static_cast<size_t>(Sequencer::NUM_SEQ_STATES)>    seq_state{ Sequencer::seq_state_names };
      StateManager<static_cast<size_t>(Sequencer::NUM_SEQ_STATES)>    req_state{ Sequencer::seq_state_names };
      StateManager<static_cast<size_t>(Sequencer::NUM_THREAD_STATES)> thread_error{ Sequencer::thread_names };
      StateManager<static_cast<size_t>(Sequencer::NUM_DAEMONS)>       daemon_ready{ Sequencer::daemon_names };

      ImprovedStateManager<static_cast<size_t>(Sequencer::NUM_SEQ_STATES)> seq_state_manager{Sequencer::seq_state_names};
      ImprovedStateManager<static_cast<size_t>(Sequencer::NUM_THREAD_STATES)> thread_state_manager{ Sequencer::thread_names };
      ImprovedStateManager<static_cast<size_t>(Sequencer::NUM_DAEMONS)>    daemon_manager{ Sequencer::daemon_names };

      std::atomic<std::uint32_t> seqstate;           ///< word to define the current state of a sequence
      std::atomic<std::uint32_t> reqstate;           ///< the currently requested state (not necc. current)

      TargetInfo target;              ///< TargetInfo object contains info for a target row and how to read it
                                      ///< Sequencer::TargetInfo is defined in sequencer_interface.h

      std::string single_obsid;       ///< obsid for single-target GETONE command
      std::string prev_single_obsid;  ///< the previous single_obsid, used for REPEAT

      std::string last_target;

      std::string tcs_which;          ///< configured TCS
      std::string tcs_name;           ///< name of TCS set on tcs initialization and shutdown

      std::string test_solver_args;   ///< optional solver args that can be passed in with a test command

      Common::Queue async;            ///< asynchronous message queue

      // Here are all the daemon client objects that the Sequencer connects to.
      //
      Common::DaemonClient acamd { "acamd" };
      Common::DaemonClient calibd { "calibd" };
      Common::DaemonClient camerad { "camerad" };
      Common::DaemonClient filterd { "filterd" };
      Common::DaemonClient flexured { "flexured" };
      Common::DaemonClient focusd { "focusd" };
      Common::DaemonClient powerd { "powerd" };
      Common::DaemonClient slicecamd { "slicecamd" };
      Common::DaemonClient slitd { "slitd" };
      Common::DaemonClient tcsd { "tcsd" };

      std::map<std::string, class PowerSwitch> power_switch;  ///< STL map of PowerSwitch objects maps all plugnames to each subsystem 

      std::vector<std::string> camera_prologue;  ///< commands to send to camera on initialization, read from cfg file
      std::vector<std::string> camera_epilogue;  ///< commands to send to camera on shutdown, read from cfg file

///   inline bool is_seqstate_set( uint32_t mb ) { return( mb & this->seqstate.load() ); }  ///< is the masked bit set in seqstate?
///   inline bool is_reqstate_set( uint32_t mb ) { return( mb & this->reqstate.load() ); }  ///< is the masked bit set in reqstate?

///   void set_seqstate_bit( uint32_t mb );     ///< set the specified masked bit in the seqstate word
      void broadcast_daemonstate();             ///<
      void broadcast_threadstate();             ///<
      void broadcast_seqstate();                ///< writes the seqstate string to the async port
      void improved_broadcast_seqstate();       ///< writes the seqstate string to the async port

      uint32_t get_reqstate();                  ///< get the reqstate word

      static void dothread_monitor_ready_state( Sequencer::Sequence &seq );

      static void dothread_sequencer_async_listener( Sequencer::Sequence &seq,
                                                     Network::UdpSocket udp ); ///< UDP ASYNC message listening thread

      static void dothread_shutdown( Sequencer::Sequence &seq );

      long startup();                                   ///< nightly startup sequence
      long shutdown( Sequencer::Sequence &seq );        ///< nightly shutdown  sequence

      bool is_ready() { return this->ready_to_start; }  ///< returns the ready_to_start state, set true only after nightly startup

      long parse_state( std::string whoami, std::string reply, bool &state );  ///< parse true|false state from reply string
      void dothread_test_fpoffset();                                           ///< for testing, calls Python function from thread
      long test( std::string args, std::string &retstring );                   ///< handles test commands
      long extract_tcs_value( std::string reply, int &value );                 ///< extract value returned by the TCS via tcsd
      long parse_tcs_generic( int value );                                     ///< parse generic TCS reply
      std::string seqstate_string( uint32_t state );                           ///< returns string form of states set in state word
      long dotype( std::string args );                                         ///< set do type (one/all)
      long dotype( std::string args, std::string &retstring );                 ///< set or get do type (one/all)
      long poll_dome_position( double &domeazi, double &telazi );
      long get_dome_position( double &domeazi, double &telazi );
      long get_dome_position( bool poll, double &domeazi, double &telazi );

      long poll_tcs_motion( std::string &state_out );
      long get_tcs_motion( std::string &state_out );
      long get_tcs_motion( bool poll, std::string &state_out );

      long get_tcs_coords_type( std::string cmd, double &ra_h, double &dec_d );///< read the current TCS ra,dec in decimal hr,deg
      long get_tcs_weather_coords( double &ra_h, double &dec_d );              ///< read the current TCS ra,dec in decimal hr,deg
      long get_tcs_coords( double &ra_h, double &dec_d );        ///< read the current TCS ra,dec,cass in decimal hr,deg
      long get_tcs_cass( double &cass );
      long tcs_init( const std::string which, std::string &retstring );  ///< initialize the specified tcs device
      long target_offset();

      void make_telemetry_message( std::string &retstring );        ///< assembles my telemetry message
      void get_external_telemetry();                                ///< collect telemetry from another daemon
      long handle_json_message( const std::string message_in );     ///< parses incoming telemetry messages

      long check_power_on( const std::string which, std::chrono::seconds delay );
      long check_connected( Common::DaemonClient &daemon );
      long check_connected( Common::DaemonClient &daemon, bool &was_opened );
      long check_connected( Common::DaemonClient &daemon, const std::string opencmd, const int opentimeout );
      long check_connected( Common::DaemonClient &daemon, const std::string opencmd, const int opentimeout, bool &was_opened );
      // These are various jobs that are done in their own threads
      //
      long trigger_exposure();       ///< trigger and wait for exposure
      void abort_process();          ///< tries to abort everything
      void stop_exposure();          ///< stop exposure timer in progress
      long repeat_exposure();        ///< repeat the last exposure
      static void dothread_modify_exptime( Sequencer::Sequence &seq, double exptime_in );  ///< modify exptime while exposure running
      void dothread_acquisition();            /// performs the acquisition sequence when signalled

      void dothread_test();
      void sequence_start();         ///< main sequence start thread
      long calib_set();              ///< sets calib according to target entry params
      long camera_set();             ///< sets camera according to target entry params
      long slit_set();               ///< sets slit according to target entry params
      long move_to_target();         ///< sends request to TCS to move to target coords
      static void dothread_notify_tcs( Sequencer::Sequence &seq );             ///< like move_to_target but for preauth only
      long focus_set();
      long flexure_set();

      long acam_init();                                        ///< initializes connection to acamd
      long calib_init();                                       ///< initializes connection to calibd
      long camera_init();                                      ///< initializes connection to camerad
      long flexure_init();                                     ///< initializes connection to flexured
      long focus_init();                                       ///< initializes connection to focusd
      long power_init();                                       ///< initializes connection to powerd
      long slicecam_init();                                    ///< initializes connection to slicecamd
      long slit_init();                                        ///< initializes connection to slitd
      long improved_tcs_init();                                         ///< initializes connection to tcsd
      static void dothread_tcs_init( Sequencer::Sequence &seq, std::string which ); ///< initializes connection to tcsd

      static void dothread_acam_shutdown( Sequencer::Sequence &seq );          ///< shutdown the acam
      static void dothread_calib_shutdown( Sequencer::Sequence &seq );         ///< shutdown the calibrator
      static void dothread_camera_shutdown( Sequencer::Sequence &seq );        ///< shutdown the camera
      static void dothread_flexure_shutdown( Sequencer::Sequence &seq );       ///< shutdown flexure system
      static void dothread_focus_shutdown( Sequencer::Sequence &seq );         ///< shutdown focusd
      static void dothread_power_shutdown( Sequencer::Sequence &seq );         ///< shutdown the power system
      static void dothread_slicecam_shutdown( Sequencer::Sequence &seq );      ///< shutdown the slicecam
      static void dothread_slit_shutdown( Sequencer::Sequence &seq );          ///< shutdown slitd
      static void dothread_tcs_shutdown( Sequencer::Sequence &seq );           ///< shutdown the TCS
  };
  /***** Sequencer::Sequence **************************************************/

}
/***** Sequencer **************************************************************/
