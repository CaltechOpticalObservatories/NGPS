/**
 * @file    calib_interface.h
 * @brief   contains the Calib::Interface class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef CALIB_INTERFACE_H
#define CALIB_INTERFACE_H

#include <map>

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "calibd_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define MOVE_TIMEOUT 25000  ///< number of milliseconds before a move fails
#define MOD_MAX          8  ///< largest allowed modulator number <n>

/***** Calib ******************************************************************/
/**
 * @namespace Calib
 * @brief     namespace for the calibrator
 *
 */
namespace Calib {

  /***** Calib::Modulator *****************************************************/
  /**
   * @class  Modulator
   * @brief  calib modulator class
   *
   * This class contains the interface to the lamp modulator.
   *
   */
  class Modulator {
    public:
      Modulator() : arduino(nullptr) { }                           ///< no default constructor

      std::unique_ptr< Network::Interface > arduino;               ///< communcate with Arduino through this interface

      /**
       * @typedef mod_info_t
       * @brief   structure of modulator info from config file
       */
      typedef struct {
        int    num;  ///< modulator number
        double dut;  ///< duty cycle
        double per;  ///< period
      } mod_info_t;

      std::map<std::string, mod_info_t> modmap_name;               ///< map of modulator info indexed by modulator name
      std::map<int, std::string> modmap_num;                       ///< map of modulator names indexed by number
      std::vector<int> mod_nums;                                   ///< vector of configured modulator numbers

      long configure_host( std::string input );                    ///< configure lamp modulator host
      long configure_mod( std::string input );                     ///< configure lamp modulators

      long open_arduino();
      long close_arduino();
      long reopen_arduino();
      long send_command( std::string cmd );                        ///< writes commands to Arduino
      long send_command( std::string cmd, std::string &reply );    ///< writes commands to, reads reply from Arduino
      long control( std::string args, std::string &retstring );    ///< lamp modulator control main parser
      long control( int num, std::string &status );                ///< lamp modulator control return status
      long control( int num, int power );                          ///< lamp modulator control set power
      long control( int num, double dut, double per );             ///< lamp modulator control set D and T
      long set_defaults();                                         ///< set all modulators as defined in config file
      long mod( int num, double dut, double per );                 ///< send command to change duty cycle and period
      long power( int num, int pow );                              ///< send command to set power state
      long status( int num, double &dut, double &per, int &pow );  ///< send command to read all status
  };
  /***** Calib::Modulator *****************************************************/


  /***** Calib::Motion ********************************************************/
  /**
   * @class  Motion
   * @brief  motion interface class for the calib hardware
   *
   * This class defines the interface for all of the calib hardware and
   * contains the functions used to communicate with it.
   *
   */
  class Motion {
    private:
      bool class_initialized;
      size_t numdev;

    public:
      std::string name;
      std::string host;
      int port;

      Motion();
      ~Motion() { };

      long configure_class();

      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long is_open( std::string arg, std::string &retstring );
      long home( std::string arg, std::string &help );    ///< home all daisy-chained motors
      static void dothread_home( Calib::Motion &motion, std::string name );
      long is_home( std::string name_in, std::string &retstring );    ///< return the home state of the motors

      long get( std::string name_in, std::string &retstring );  ///< get state of named actuator(s)
      long set( std::string input, std::string &retstring );    ///< set state(s) of named actuator(s)
      long send_command( const std::string &name, std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( const std::string &name, std::string cmd, std::string &retstring );  ///< writes command?, reads reply

      // PI Interface class of the Servo type
      //
      Physik_Instrumente::Interface<Physik_Instrumente::ServoInfo> motorinterface;

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads
      std::mutex wait_mtx;                       ///< mutex object for waiting for threads
      std::condition_variable cv;                ///< condition variable for waiting for threads

  };
  /***** Calib::Motion ********************************************************/


  /***** Calib::Interface *****************************************************/
  /**
   * @class  Interface
   * @brief  main interface class for the calib hardware
   *
   * This class defines the interface for all of the calib hardware and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    public:
      Interface()  { };
      ~Interface() { };

      Common::Queue async;                       ///< asynchronous message queue object

      Motion motion;                             ///< motion object

      Modulator modulator;                       ///< lamp modulator object

      void make_telemetry_message( std::string &retstring );  ///< assembles telemetry message
  };
  /***** Calib::Interface *****************************************************/

}
/***** Calib ******************************************************************/
#endif
