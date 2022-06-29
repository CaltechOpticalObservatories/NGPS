/** ---------------------------------------------------------------------------
 * @file     sequence.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
 *
 */

#include "sequencerd.h"
#include "slitd_commands.h"

#ifndef SEQUENCE_H
#define SEQUENCE_H

namespace Sequencer {

  // run states are global
  //
  const int STOPPED = 0;  /// run state stopped
  const int RUNNING = 1;  /// run state running

  enum SequenceStateBits {  /// assigns each subsystem a bit in the seqstate word
    READY_BIT=0,
    SLIT_BIT,
    CALIB_BIT,
    FOCUS_BIT,
    FLEXURE_BIT,
    TCS_BIT
  };

  const std::uint32_t SEQ_STOPPED      = 0;
  const std::uint32_t SEQ_READY        = 1 << READY_BIT;
  const std::uint32_t SEQ_WAIT_SLIT    = 1 << SLIT_BIT;
  const std::uint32_t SEQ_WAIT_CALIB   = 1 << CALIB_BIT;
  const std::uint32_t SEQ_WAIT_TCS     = 1 << TCS_BIT;
  const std::uint32_t SEQ_WAIT_FOCUS   = 1 << FOCUS_BIT;
  const std::uint32_t SEQ_WAIT_FLEXURE = 1 << FLEXURE_BIT;

  class Sequence {
    private:
      bool ready_to_start;
    public:
      Sequence();
      ~Sequence();

      std::mutex start_mtx;   /// mutex to protect the start sequence from multiple instances

      std::mutex wait_mtx;
      std::condition_variable cv;

      volatile std::atomic<std::uint32_t> runstate;
      volatile std::atomic<std::uint32_t> seqstate;  /// word to define the state of a sequence

      TargetInfo target;

      Daemon camerad;
      Daemon flexured;
      Daemon calibd;
      Daemon slitd;
      Daemon focusd;
      Daemon powerd;
      Daemon tcsd;

      void set_seqstate_bit( uint32_t state );  /// set the specified bit in the seqstate word
      void clr_seqstate_bit( uint32_t state );  /// clear the specified bit in the seqstate word

      long startup();
      bool is_ready() { return this->ready_to_start; }

      long get_next_target();

      // These are various jobs that are done in their own threads
      //
      static void dothread_wait_for_state( Sequencer::Sequence &seq, uint32_t state );  /// wait for seqstate to be requested state
      static void dothread_sequence_start( Sequencer::Sequence &seq );                  /// main sequence start thread
      static void dothread_set_slit( Sequencer::Sequence &seq );                        /// sets slit according to target entry params
      static void dothread_acquire_target( Sequencer::Sequence &seq );                  /// 
      static void dothread_set_focus( Sequencer::Sequence &seq );
      static void dothread_set_flexure( Sequencer::Sequence &seq );
      static void dothread_set_calibrator( Sequencer::Sequence &seq );

  };

}
#endif
