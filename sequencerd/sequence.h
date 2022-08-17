/** ---------------------------------------------------------------------------
 * @file     sequence.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
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

  // run states are global
  //
  const int STOPPED = 0;  /// run state stopped
  const int RUNNING = 1;  /// run state running

  enum SequenceStateBits {  /// assigns each subsystem a bit in the seqstate word
    STOP_BIT=0,
    READY_BIT,
    CALIB_BIT,
    CAMERA_BIT,
    FILTER_BIT,
    FLEXURE_BIT,
    FOCUS_BIT,
    POWER_BIT,
    SLIT_BIT,
    TCS_BIT
  };

  const std::uint32_t SEQ_STOPPED      = 0;                 /// all bits clear when not running
  const std::uint32_t SEQ_STOP         = 1 << STOP_BIT;     /// set when a stop is requested
  const std::uint32_t SEQ_READY        = 1 << READY_BIT;    /// set when ready to proceed
  const std::uint32_t SEQ_WAIT_CALIB   = 1 << CALIB_BIT;    /// set when waiting for calib
  const std::uint32_t SEQ_WAIT_CAMERA  = 1 << CAMERA_BIT;   /// set when waiting for camera
  const std::uint32_t SEQ_WAIT_FILTER  = 1 << FILTER_BIT;   /// set when waiting for filter
  const std::uint32_t SEQ_WAIT_FLEXURE = 1 << FLEXURE_BIT;  /// set when waiting for flexure
  const std::uint32_t SEQ_WAIT_FOCUS   = 1 << FOCUS_BIT;    /// set when waiting for focus
  const std::uint32_t SEQ_WAIT_POWER   = 1 << POWER_BIT;    /// set when waiting for power
  const std::uint32_t SEQ_WAIT_SLIT    = 1 << SLIT_BIT;     /// set when waiting for slit
  const std::uint32_t SEQ_WAIT_TCS     = 1 << TCS_BIT;      /// set when waiting for tcs

  class Sequence {
    private:
      bool ready_to_start;    /// is the system ready to start? set true only after nightly startup is successful
    public:
      Sequence();
      ~Sequence() { };

      double tcs_timeout;     /// timeout for telescope moves (in sec) set by configuration parameter TCS_TIMEOUT
      std::mutex start_mtx;   /// mutex to protect the start sequence from multiple instances

      std::mutex wait_mtx;
      std::condition_variable cv;

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

      void set_seqstate_bit( uint32_t state );  /// set the specified bit in the seqstate word
      void clr_seqstate_bit( uint32_t state );  /// clear the specified bit in the seqstate word
      uint32_t get_seqstate();                  /// get the seqstate word
      uint32_t get_reqstate();                  /// get the reqstate word

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

      // These are various jobs that are done in their own threads
      //
      static void dothread_trigger_exposure( Sequencer::Sequence &seq );       /// trigger and wait for exposure

      static void dothread_wait_for_state( Sequencer::Sequence &seq );         /// wait for seqstate to be requested state
      static void dothread_sequence_start( Sequencer::Sequence &seq );         /// main sequence start thread
      static void dothread_calib_set( Sequencer::Sequence &seq );              /// sets calib according to target entry params
      static void dothread_camera_set( Sequencer::Sequence &seq );             /// sets camera according to target entry params
      static void dothread_slit_set( Sequencer::Sequence &seq );               /// sets slit according to target entry params
      static void dothread_acquire_target( Sequencer::Sequence &seq );         /// 
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
