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

#include <atomic>
#include <cmath>
#include <mysqlx/xdevapi.h>
#include <json.hpp>

#include "sequencer_interface.h"  // this defines the classes used to interface with various subsystems

#include "acamd_commands.h"
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

const int foo=2;
  /**
   * @enum  SequenceStateBits
   * @brief assigns each subsystem a bit in the seqstate word
   */
  enum SequenceStateBits : int {
    OFFLINE_BIT=0,
    ABORTREQ_BIT,
    STOPREQ_BIT,
    READY_BIT,
    RUNNING_BIT,
    SHUTTING_BIT,
    PAUSE_BIT,
    STARTING_BIT,
    ACAM_BIT,
    ACQUIRE_BIT,
    CALIB_BIT,
    CAMERA_BIT,
    EXPOSE_BIT,
    GUIDE_BIT,
    READOUT_BIT,
    FILTER_BIT,
    FLEXURE_BIT,
    FOCUS_BIT,
    POWER_BIT,
    SLIT_BIT,
    TCS_BIT,
    TCSOP_BIT,
    SETTLE_BIT,
    SLEW_BIT,
    NUM_STATE_BITS          /// NUM_STATE_BITS used for internal error checking
  };

  const std::uint32_t SEQ_OFFLINE      = 1 << OFFLINE_BIT;   ///< set when when offline
  const std::uint32_t SEQ_ABORTREQ     = 1 << ABORTREQ_BIT;  ///< set when an abort is requested
  const std::uint32_t SEQ_STOPREQ      = 1 << STOPREQ_BIT;   ///< set when a stop is requested
  const std::uint32_t SEQ_READY        = 1 << READY_BIT;     ///< set when sequencer is ready to be started
  const std::uint32_t SEQ_RUNNING      = 1 << RUNNING_BIT;   ///< set when sequencer is running
  const std::uint32_t SEQ_SHUTTING     = 1 << SHUTTING_BIT;  ///< set when sequencer is shutting down
  const std::uint32_t SEQ_PAUSE        = 1 << PAUSE_BIT;     ///< set when sequencer is paused
  const std::uint32_t SEQ_STARTING     = 1 << STARTING_BIT;  ///< set when sequencer is starting up
  const std::uint32_t SEQ_WAIT_ACAM    = 1 << ACAM_BIT;      ///< set when waiting for acam
  const std::uint32_t SEQ_WAIT_ACQUIRE = 1 << ACQUIRE_BIT;   ///< set when waiting for acquire
  const std::uint32_t SEQ_WAIT_CALIB   = 1 << CALIB_BIT;     ///< set when waiting for calib
  const std::uint32_t SEQ_WAIT_CAMERA  = 1 << CAMERA_BIT;    ///< set when waiting for camera
  const std::uint32_t SEQ_WAIT_EXPOSE  = 1 << EXPOSE_BIT;    ///< set when waiting for camera exposure
  const std::uint32_t SEQ_GUIDE        = 1 << GUIDE_BIT;     ///< set/clear to enable/disable guide thread
  const std::uint32_t SEQ_WAIT_READOUT = 1 << READOUT_BIT;   ///< set when waiting for camera readout
  const std::uint32_t SEQ_WAIT_FILTER  = 1 << FILTER_BIT;    ///< set when waiting for filter
  const std::uint32_t SEQ_WAIT_FLEXURE = 1 << FLEXURE_BIT;   ///< set when waiting for flexure
  const std::uint32_t SEQ_WAIT_FOCUS   = 1 << FOCUS_BIT;     ///< set when waiting for focus
  const std::uint32_t SEQ_WAIT_POWER   = 1 << POWER_BIT;     ///< set when waiting for power
  const std::uint32_t SEQ_WAIT_SLIT    = 1 << SLIT_BIT;      ///< set when waiting for slit
  const std::uint32_t SEQ_WAIT_TCS     = 1 << TCS_BIT;       ///< set when waiting for tcs
  const std::uint32_t SEQ_WAIT_TCSOP   = 1 << TCSOP_BIT;     ///< set when waiting specifically for tcs operator
  const std::uint32_t SEQ_WAIT_SETTLE  = 1 << SETTLE_BIT;    ///< set when waiting specifically for tcs settle
  const std::uint32_t SEQ_WAIT_SLEW    = 1 << SLEW_BIT;      ///< set when waiting specifically for tcs slew

  /**
   * @enum  ThreadStatusBits
   * @brief assigns each thread a bit in a threadstate word
   */
  enum ThreadStatusBits : int {
    ASYNCLISTENER_BIT=0,
    TRIGEXPO_BIT,
    WAITFORSTATE_BIT,
    SEQSTART_BIT,
    MONITORREADY_BIT,
    CALIBSET_BIT,
    CAMERASET_BIT,
    SLITSET_BIT,
    MOVETOTARGET_BIT,
    NOTIFYTCS_BIT,
    FOCUSSET_BIT,
    FLEXURESET_BIT,
    CALIBRATORSET_BIT,
    ACAMINIT_BIT,
    CALIBINIT_BIT,
    TCSINIT_BIT,
    SLITINIT_BIT,
    CAMERAINIT_BIT,
    FLEXUREINIT_BIT,
    FOCUSINIT_BIT,
    POWERINIT_BIT,
    ACAMSTOP_BIT,
    CALIBSTOP_BIT,
    TCSSTOP_BIT,
    POWERSTOP_BIT,
    MODEXPTIME_BIT,
    ACQUISITION_BIT,
    GUIDING_BIT,
    STARTUP_BIT,
    SHUTDOWN_BIT,
    NUM_THREAD_STATE_BITS
  };

  const std::uint32_t THR_NONE                     = 0;                       ///< used to clear anything thread related
  const std::uint32_t THR_SEQUENCER_ASYNC_LISTENER = 1 << ASYNCLISTENER_BIT;  ///< set when dothread_sequencer_async_listener running
  const std::uint32_t THR_TRIGGER_EXPOSURE         = 1 << TRIGEXPO_BIT;       ///< set when dothread_trigger_exposure running
  const std::uint32_t THR_WAIT_FOR_STATE           = 1 << WAITFORSTATE_BIT;   ///< set when dothread_wait_for_state running
  const std::uint32_t THR_SEQUENCE_START           = 1 << SEQSTART_BIT;       ///< set when dothread_sequencer_start running
  const std::uint32_t THR_MONITOR_READY_STATE      = 1 << MONITORREADY_BIT;
  const std::uint32_t THR_CALIB_SET                = 1 << CALIBSET_BIT;
  const std::uint32_t THR_CAMERA_SET               = 1 << CAMERASET_BIT;
  const std::uint32_t THR_SLIT_SET                 = 1 << SLITSET_BIT;
  const std::uint32_t THR_MOVE_TO_TARGET           = 1 << MOVETOTARGET_BIT;
  const std::uint32_t THR_NOTIFY_TCS               = 1 << NOTIFYTCS_BIT;
  const std::uint32_t THR_FOCUS_SET                = 1 << FOCUSSET_BIT;
  const std::uint32_t THR_FLEXURE_SET              = 1 << FLEXURESET_BIT;
  const std::uint32_t THR_CALIBRATOR_SET           = 1 << CALIBRATORSET_BIT;
  const std::uint32_t THR_ACAM_INIT                = 1 << ACAMINIT_BIT;
  const std::uint32_t THR_CALIB_INIT               = 1 << CALIBINIT_BIT;
  const std::uint32_t THR_TCS_INIT                 = 1 << TCSINIT_BIT;
  const std::uint32_t THR_SLIT_INIT                = 1 << SLITINIT_BIT;
  const std::uint32_t THR_CAMERA_INIT              = 1 << CAMERAINIT_BIT;
  const std::uint32_t THR_FLEXURE_INIT             = 1 << FLEXUREINIT_BIT;
  const std::uint32_t THR_FOCUS_INIT               = 1 << FOCUSINIT_BIT;
  const std::uint32_t THR_POWER_INIT               = 1 << POWERINIT_BIT;
  const std::uint32_t THR_ACAM_SHUTDOWN            = 1 << ACAMSTOP_BIT;
  const std::uint32_t THR_CALIB_SHUTDOWN           = 1 << CALIBSTOP_BIT;
  const std::uint32_t THR_TCS_SHUTDOWN             = 1 << TCSSTOP_BIT;
  const std::uint32_t THR_POWER_SHUTDOWN           = 1 << POWERSTOP_BIT;
  const std::uint32_t THR_MODIFY_EXPTIME           = 1 << MODEXPTIME_BIT;
  const std::uint32_t THR_ACQUISITION              = 1 << ACQUISITION_BIT;
  const std::uint32_t THR_GUIDING                  = 1 << GUIDING_BIT;
  const std::uint32_t THR_STARTUP                  = 1 << STARTUP_BIT;
  const std::uint32_t THR_SHUTDOWN                 = 1 << SHUTDOWN_BIT;


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
    public:
      Sequence();
      ~Sequence() { };

      std::map<std::string, int> telemetry_providers;  ///< map of port[daemon_name] for external telemetry providers

      double acquisition_timeout; ///< timeout for target acquisition (in sec) set by configuration parameter ACAM_ACQUIRE_TIMEOUT
      int acquisition_max_retrys; ///< max number of acquisition loop attempts
      double tcs_offsetrate_ra;   ///< TCS offset rate RA ("MRATE") in arcsec per second
      double tcs_offsetrate_dec;  ///< TCS offset rate DEC ("MRATE") in arcsec per second
      double tcs_settle_timeout;  ///< timeout for telescope to settle (in sec) set by configuration parameter TCS_SETTLE_TIMEOUT
      double tcs_settle_stable;   ///< time that TCS must report TRACKING before it is really tracking
      double tcs_domeazi_ready;   ///< max degrees azimuth that dome and telescope can differ before ready to observe
      double tcs_preauth_time;    ///< seconds before end of exposure to notify TCS of next target's coords (0 to disable)
      std::mutex start_mtx;       ///< mutex to protect the start sequence from multiple instances

      std::mutex wait_mtx;
      std::condition_variable cv;
      std::mutex monitor_mtx;

      std::map<int, std::string> sequence_states;
      std::vector<int> sequence_state_bits;

      std::map<int, std::string> thread_states;
      std::vector<int> thread_state_bits;

      volatile std::atomic<bool>          waiting_for_state;  ///< set if dothread_wait_for_state is running
      volatile std::atomic<bool>          do_once;            ///< set if "do one" selected, clear if "do all" selected
      volatile std::atomic<bool>          tcs_ontarget;       ///< remotely set by the TCS operator to indicate that the target is ready
      volatile std::atomic<bool>          tcs_nowait;         ///< set to skip waiting for TCS
      volatile std::atomic<bool>          dome_nowait;        ///< set to skip waiting for dome

      volatile std::atomic<std::uint32_t> thrstate;           ///< word to indicate which threads are running
      volatile std::atomic<std::uint32_t> seqstate;           ///< word to define the current state of a sequence
      volatile std::atomic<std::uint32_t> reqstate;           ///< the currently requested state (not necc. current)
      volatile std::atomic<std::uint32_t> system_not_ready;   ///< set bits indicate which subsystem is not ready

      volatile std::atomic<std::uint32_t> thr_error;          ///< error state of threads
      volatile std::atomic<std::uint32_t> thr_which_err;      ///< word to define which thread caused an error

      TargetInfo target;              ///< TargetInfo object contains info for a target row and how to read it
                                      ///< Sequencer::TargetInfo is defined in sequencer_interface.h

      std::string last_target;

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
      Common::DaemonClient slitd { "slitd" };
      Common::DaemonClient tcsd { "tcsd" };

      std::map<std::string, class PowerSwitch> power_switch;  ///< STL map of PowerSwitch objects maps all plugnames to each subsystem 

      std::vector<std::string> camera_preamble;  ///< commands to send to camera on initialization, read from cfg file

      inline bool is_seqstate_set( uint32_t mb ) { return( mb & this->seqstate.load() ); }  ///< is the masked bit set in seqstate?
      inline bool is_reqstate_set( uint32_t mb ) { return( mb & this->reqstate.load() ); }  ///< is the masked bit set in reqstate?

      void set_seqstate_bit( uint32_t mb );     ///< set the specified masked bit in the seqstate word
      void clr_seqstate_bit( uint32_t mb );     ///< clear the specified masked bit in the seqstate word
      void set_clr_seqstate_bit( uint32_t sb, uint32_t cb );     ///< set and clear the specified masked bit(s) in the seqstate word
      std::string report_seqstate();            ///< writes the seqstate string to the async port
      uint32_t get_seqstate();                  ///< get the seqstate word

      void set_reqstate_bit( uint32_t mb );     ///< set the specified masked bit in the reqstate word
      void clr_reqstate_bit( uint32_t mb );     ///< clear the specified masked bit in the reqstate word
      uint32_t get_reqstate();                  ///< get the reqstate word

      inline void set_thrstate_bit( uint32_t mb ) { this->thrstate.fetch_or( mb ); }
      inline void clr_thrstate_bit( uint32_t mb ) { this->thrstate.fetch_and( ~mb ); }

      static void dothread_monitor_ready_state( Sequencer::Sequence &seq );

      static void dothread_sequencer_async_listener( Sequencer::Sequence &seq,
                                                     Network::UdpSocket udp ); ///< UDP ASYNC message listening thread

      static void dothread_startup( Sequencer::Sequence &seq );
      static void dothread_shutdown( Sequencer::Sequence &seq );

      long startup( Sequencer::Sequence &seq );         ///< nightly startup sequence
      long shutdown( Sequencer::Sequence &seq );        ///< nightly shutdown  sequence

      bool is_ready() { return this->ready_to_start; }  ///< returns the ready_to_start state, set true only after nightly startup

      long parse_state( std::string whoami, std::string reply, bool &state );  ///< parse true|false state from reply string
      void dothread_test_fpoffset();                                           ///< for testing, calls Python function from thread
      long test( std::string args, std::string &retstring );                   ///< handles test commands
      long extract_tcs_value( std::string reply, int &value );                 ///< extract value returned by the TCS via tcsd
      long parse_tcs_generic( int value );                                     ///< parse generic TCS reply
      std::string seqstate_string( uint32_t state );                           ///< returns string form of states set in state word
      std::string thrstate_string( uint32_t state );                           ///< returns string form of thread states
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

      void make_telemetry_message( std::string &retstring );        ///< assembles my telemetry message
      void get_external_telemetry();                                ///< collect telemetry from another daemon
      long handle_json_message( const std::string message_in );     ///< parses incoming telemetry messages

      // These are various jobs that are done in their own threads
      //
      static void dothread_trigger_exposure( Sequencer::Sequence &seq );       ///< trigger and wait for exposure
      static void dothread_modify_exptime( Sequencer::Sequence &seq, double exptime_in );  ///< modify exptime while exposure running
      static void dothread_acquisition( Sequencer::Sequence &seq );            /// performs the acquisition sequence when signalled

      static void dothread_wait_for_state( Sequencer::Sequence &seq );         ///< wait for seqstate to be requested state
      static void dothread_sequence_start( Sequencer::Sequence &seq );         ///< main sequence start thread
      static void dothread_calib_set( Sequencer::Sequence &seq );              ///< sets calib according to target entry params
      static void dothread_camera_set( Sequencer::Sequence &seq );             ///< sets camera according to target entry params
      static void dothread_slit_set( Sequencer::Sequence &seq );               ///< sets slit according to target entry params
      static void dothread_move_to_target( Sequencer::Sequence &seq );         ///< sends request to TCS to move to target coords
      static void dothread_notify_tcs( Sequencer::Sequence &seq );             ///< like move_to_target but for preauth only
      static void dothread_focus_set( Sequencer::Sequence &seq );
      static void dothread_flexure_set( Sequencer::Sequence &seq );

      static void dothread_acam_init( Sequencer::Sequence &seq );              ///< initializes connection to acamd
      static void dothread_calib_init( Sequencer::Sequence &seq );             ///< initializes connection to calibd
      static void dothread_tcs_init( Sequencer::Sequence &seq, std::string which ); ///< initializes connection to tcsd
      static void dothread_slit_init( Sequencer::Sequence &seq );              ///< initializes connection to slitd
      static void dothread_camera_init( Sequencer::Sequence &seq );            ///< initializes connection to camerad
      static void dothread_flexure_init( Sequencer::Sequence &seq );           ///< initializes connection to flexured
      static void dothread_focus_init( Sequencer::Sequence &seq );             ///< initializes connection to focusd
      static void dothread_power_init( Sequencer::Sequence &seq );             ///< initializes connection to powerd

      static void dothread_tcs_shutdown( Sequencer::Sequence &seq );           ///< shutdown the TCS
      static void dothread_calib_shutdown( Sequencer::Sequence &seq );         ///< shutdown the calibrator
      static void dothread_acam_shutdown( Sequencer::Sequence &seq );          ///< shutdown the acam
      static void dothread_power_shutdown( Sequencer::Sequence &seq );         ///< shutdown the power system
  };
  /***** Sequencer::Sequence **************************************************/

}
/***** Sequencer **************************************************************/
