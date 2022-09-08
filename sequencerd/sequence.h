/**
 * @file     sequence.h
 * @brief    header file for the Sequence class which contains the sequence functions
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 * Ths Sequencer::Sequence class contains functions which perform the 
 * sequence operations which are called by the sequencer daemon.
 *
 */

#include <mysqlx/xdevapi.h>

#include "sequencerd.h"
#include "slitd_commands.h"
#include "calibd_commands.h"
#include "tcsd_commands.h"
#include "camerad_commands.h"
#include "flexured_commands.h"
#include "focusd_commands.h"
#include "powerd_commands.h"

#include "tcs_constants.h"

#ifndef SEQUENCE_H
#define SEQUENCE_H

namespace Sequencer {

  enum SequenceStateBits : int {  /// SequenceStateBits assigns each subsystem a bit in the seqstate word
    STOPPED_BIT=0,
    ABORTING_BIT,
    STOPREQ_BIT,
    STARTING_BIT,
    READY_BIT,
    RUNNING_BIT,
    CALIB_BIT,
    CAMERA_BIT,
    FILTER_BIT,
    FLEXURE_BIT,
    FOCUS_BIT,
    POWER_BIT,
    SLIT_BIT,
    TCS_BIT,
    NUM_STATE_BITS          /// NUM_STATE_BITS used for internal error checking
  };

  const std::uint32_t SEQ_STOPPED      = 1 << STOPPED_BIT;  /// all bits clear when stopped
  const std::uint32_t SEQ_ABORTING     = 1 << ABORTING_BIT; /// set when an abort is requested
  const std::uint32_t SEQ_STOPREQ      = 1 << STOPREQ_BIT;  /// set when a stop is requested
  const std::uint32_t SEQ_STARTING     = 1 << STARTING_BIT; /// set when sequencer is starting up
  const std::uint32_t SEQ_READY        = 1 << READY_BIT;    /// set when sequencer is ready to be started
  const std::uint32_t SEQ_RUNNING      = 1 << RUNNING_BIT;  /// set when sequencer is running
  const std::uint32_t SEQ_WAIT_CALIB   = 1 << CALIB_BIT;    /// set when waiting for calib
  const std::uint32_t SEQ_WAIT_CAMERA  = 1 << CAMERA_BIT;   /// set when waiting for camera
  const std::uint32_t SEQ_WAIT_FILTER  = 1 << FILTER_BIT;   /// set when waiting for filter
  const std::uint32_t SEQ_WAIT_FLEXURE = 1 << FLEXURE_BIT;  /// set when waiting for flexure
  const std::uint32_t SEQ_WAIT_FOCUS   = 1 << FOCUS_BIT;    /// set when waiting for focus
  const std::uint32_t SEQ_WAIT_POWER   = 1 << POWER_BIT;    /// set when waiting for power
  const std::uint32_t SEQ_WAIT_SLIT    = 1 << SLIT_BIT;     /// set when waiting for slit
  const std::uint32_t SEQ_WAIT_TCS     = 1 << TCS_BIT;      /// set when waiting for tcs

  enum ThreadStatusBits : int {   /// ThreadStatusBits assigns each thread a bit in a threadstate word
    ASYNCLISTENER_BIT=0,
    TRIGEXPO_BIT,
    WAITFORSTATE_BIT,
    SEQSTART_BIT,
    CALIBSET_BIT,
    CAMERASET_BIT,
    SLITSET_BIT,
    MOVETOTARGET_BIT,
    NOTIFYTCS_BIT,
    FOCUSSET_BIT,
    FLEXURESET_BIT,
    CALIBRATORSET_BIT,
    CALIBINIT_BIT,
    TCSINIT_BIT,
    SLITINIT_BIT,
    CAMERAINIT_BIT,
    FLEXUREINIT_BIT,
    FOCUSINIT_BIT,
    POWERINIT_BIT,
    NUM_THREAD_STATE_BITS
  };

  const std::uint32_t THR_SEQUENCE_ASYNC_LISTENER = 1 << ASYNCLISTENER_BIT;
  const std::uint32_t THR_TRIGGER_EXPOSURE        = 1 << TRIGEXPO_BIT;
  const std::uint32_t THR_WAIT_FOR_STATE          = 1 << WAITFORSTATE_BIT;
  const std::uint32_t THR_SEQUENCE_START          = 1 << SEQSTART_BIT;
  const std::uint32_t THR_CALIB_SET               = 1 << CALIBSET_BIT;
  const std::uint32_t THR_CAMERA_SET              = 1 << CAMERASET_BIT;
  const std::uint32_t THR_SLIT_SET                = 1 << SLITSET_BIT;
  const std::uint32_t THR_MOVE_TO_TARGET          = 1 << MOVETOTARGET_BIT;
  const std::uint32_t THR_NOTIFY_TCS              = 1 << NOTIFYTCS_BIT;
  const std::uint32_t THR_FOCUS_SET               = 1 << FOCUSSET_BIT;
  const std::uint32_t THR_FLEXURE_SET             = 1 << FLEXURESET_BIT;
  const std::uint32_t THR_CALIBRATOR_SET          = 1 << CALIBRATORSET_BIT;
  const std::uint32_t THR_CALIB_INIT              = 1 << CALIBINIT_BIT;
  const std::uint32_t THR_TCS_INIT                = 1 << TCSINIT_BIT;
  const std::uint32_t THR_SLIT_INIT               = 1 << SLITINIT_BIT;
  const std::uint32_t THR_CAMERA_INIT             = 1 << CAMERAINIT_BIT;
  const std::uint32_t THR_FLEXURE_INIT            = 1 << FLEXUREINIT_BIT;
  const std::uint32_t THR_FOCUS_INIT              = 1 << FOCUSINIT_BIT;
  const std::uint32_t THR_POWER_INIT              = 1 << POWERINIT_BIT;

  class Sequence {
    private:
      bool ready_to_start;                       /// set on nightly startup success, used to return seqstate to READY after an abort
      std::atomic<bool> notify_tcs_next_target;  /// notify TCS of next target when remaining time within TCS_PREAUTH_TIME
    public:
      Sequence();
      ~Sequence() { };

      double tcs_settle_timeout;  /// timeout for telescope to settle (in sec) set by configuration parameter TCS_SETTLE_TIMEOUT
      double tcs_preauth_time;    /// seconds before end of exposure to notify TCS of next target's coords (0 to disable)
      std::mutex start_mtx;       /// mutex to protect the start sequence from multiple instances

      std::mutex wait_mtx;
      std::condition_variable cv;

      std::map<int, std::string> sequence_states;
      std::vector<int> sequence_state_bits;

      std::map<int, std::string> thread_states;
      std::vector<int> thread_state_bits;

      volatile std::atomic<bool>          waiting_for_state;   /// set if dothread_wait_for_state is running

      volatile std::atomic<std::uint32_t> thrstate;  /// word to indicate which threads are running
      volatile std::atomic<std::uint32_t> runstate;
      volatile std::atomic<std::uint32_t> seqstate;  /// word to define the current state of a sequence
      volatile std::atomic<std::uint32_t> reqstate;  /// the currently requested state (not necc. current)

      volatile std::atomic<long>          thr_error;      /// error state of threads
      volatile std::atomic<std::uint32_t> thr_which_err;  /// word to define which thread caused an error

      TargetInfo target;              /// TargetInfo object contains info for a target row and how to read it

      std::string last_target_name;   /// remember the last target observed to prevent uneccessary slews

      // Here are all the daemon objects that the Sequencer connects to
      //
      Daemon calibd;
      Daemon camerad;
      Daemon filterd;
      Daemon flexured;
      Daemon focusd;
      Daemon powerd;
      Daemon slitd;
      Daemon tcsd;

      inline bool is_seqstate_set( uint32_t mb ) { return( mb & this->seqstate.load() ); }  /// is the masked bit set in seqstate?

      void set_seqstate_bit( uint32_t mb );     /// set the specified masked bit in the seqstate word
      void clr_seqstate_bit( uint32_t mb );     /// clear the specified masked bit in the seqstate word
      uint32_t get_seqstate();                  /// get the seqstate word

      void set_reqstate_bit( uint32_t mb );     /// set the specified masked bit in the reqstate word
      void clr_reqstate_bit( uint32_t mb );     /// clear the specified masked bit in the reqstate word
      uint32_t get_reqstate();                  /// get the reqstate word

      inline void set_thrstate_bit( uint32_t mb ) { this->thrstate.fetch_or( mb ); }
      inline void clr_thrstate_bit( uint32_t mb ) { this->thrstate.fetch_and( ~mb ); }

      static void dothread_sequence_async_listener( Sequencer::Sequence &seq,
                                                    Network::UdpSocket udp );  /// UDP ASYNC message listening thread

      long startup( Sequencer::Sequence &seq );         /// nightly startup sequence
      bool is_ready() { return this->ready_to_start; }  /// returns the ready_to_start state, set true only after nightly startup

      long parse_state( std::string whoami, std::string reply, bool &state );  /// parse true|false state from reply string
      double radec_to_decimal( std::string str_in );                           /// convert ra,dec from string to double
      double radec_to_decimal( std::string str_in, std::string &retstring );   /// convert ra,dec from string to double
      long test( std::string args, std::string &retstring );                   /// handles test commands
      long extract_tcs_value( std::string reply, int &value );                 /// extract value returned by the TCS via tcsd
      long parse_tcs_generic( int value );                                     /// parse generic TCS reply
      std::string seqstate_string( uint32_t state );                           /// returns string form of states set in state word
      std::string thrstate_string( uint32_t state );                           /// returns string form of thread states

      // These are various jobs that are done in their own threads
      //
      static void dothread_trigger_exposure( Sequencer::Sequence &seq );       /// trigger and wait for exposure

      static void dothread_wait_for_state( Sequencer::Sequence &seq );         /// wait for seqstate to be requested state
      static void dothread_sequence_start( Sequencer::Sequence &seq );         /// main sequence start thread
      static void dothread_calib_set( Sequencer::Sequence &seq );              /// sets calib according to target entry params
      static void dothread_camera_set( Sequencer::Sequence &seq );             /// sets camera according to target entry params
      static void dothread_slit_set( Sequencer::Sequence &seq );               /// sets slit according to target entry params
      static void dothread_move_to_target( Sequencer::Sequence &seq );         /// sends request to TCS to move to target coords
      static void dothread_notify_tcs( Sequencer::Sequence &seq );             /// like move_to_target but for preauth only
      static void dothread_focus_set( Sequencer::Sequence &seq );
      static void dothread_flexure_set( Sequencer::Sequence &seq );
      static void dothread_calibrator_set( Sequencer::Sequence &seq );

      static void dothread_calib_init( Sequencer::Sequence &seq );             /// initializes connection to calibd
      static void dothread_tcs_init( Sequencer::Sequence &seq );               /// initializes connection to tcsd
      static void dothread_slit_init( Sequencer::Sequence &seq );              /// initializes connection to slitd
      static void dothread_camera_init( Sequencer::Sequence &seq );            /// initializes connection to camerad
      static void dothread_flexure_init( Sequencer::Sequence &seq );           /// initializes connection to flexured
      static void dothread_focus_init( Sequencer::Sequence &seq );             /// initializes connection to focusd
      static void dothread_power_init( Sequencer::Sequence &seq );             /// initializes connection to powerd
  };

}
#endif
