/** ---------------------------------------------------------------------------
 * @file    tcs_interface.h
 * @brief   tcs interface include
 * @details defines the classes used by the tcs hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "network.h"
#include "logentry.h"
#include "common.h"
#include "tcs_constants.h"
#include "tcsd_commands.h"
#include <sys/stat.h>
#include <map>
#include <memory>
#include <mutex>
#include <math.h>

/***** TCS ********************************************************************/
/**
 * @namespace TCS
 * @brief     namespace for the TCS daemon
 *
 */
namespace TCS {

  constexpr bool BLOCK=true;

  /***** TCS::TcsInfo *********************************************************/
  /**
   * @class   TcsInfo
   * @brief   TCS information class
   * @details This class contains functions which can parse the output from
   *          TCS-native commands REQSTAT, REQPOS, and ?WEATHER, which then
   *          store the parsed results into class variables.
   *
   */
  class TcsInfo {
    public:
      std::string utc;      /// ddd hh:mm:ss
      std::string lst;      /// hh:mm:ss
      std::string ha;       /// hh:mm:ss.s
      std::string ra_hms;   /// hh:mm:ss.ss
      std::string dec_dms;  /// dd:mm:ss.ss

      double ra_h_dec;      /// h.hhhhh (decimal hours)
      double dec_d_dec;     /// d.ddddd (decimal degrees)
      double azimuth;
      double zenithangle;
      double domeazimuth;

      double airmass;
      double focus;
      double offsetra;
      double offsetdec;
      double offsetrate_ra;
      double offsetrate_dec;
      double cassangle;

      int domeshutters;

      TcsInfo() { this->init(); }

      /**
       * @brief  initialize all class member variables to "non-values"
       */
      void init() {
        utc.clear();
        lst.clear();
        ha.clear();
        ra_hms.clear();
        dec_dms.clear();
        ra_h_dec=NAN;
        dec_d_dec=NAN;
        azimuth=NAN;
        zenithangle=NAN;
        domeazimuth=NAN;
        airmass=NAN;
        focus=NAN;
        offsetrate_ra=NAN;
        offsetrate_dec=NAN;
        cassangle=NAN;
        domeshutters=-1;
      }

      // These functions parse the return string from native TCS commands
      // and store the results in the class variables.
      //
      void parse_weather( std::string &input );  ///< parse retstring from native ?WEATHER
      void parse_reqstat( std::string &input );  ///< parse retstring from native REQSTAT
      void parse_reqpos( std::string &input );   ///< parse retstring from native REQPOS
  };
  /***** TCS::TcsInfo *********************************************************/


  /***** TCS::TcsIO ***********************************************************/
  /**
   * @class   TcsIO
   * @brief   interface class provides the actual I/O for a TCS device
   * @details There are (at least) two TCS "devices", the real and simulated.
   *          For safety, the client must specify which TCS is to be used,
   *          which prevents a casual "open" command from accidentally opening
   *          the real TCS when the simulated was intended.
   *
   * This class performs the I/O with the TCS.
   *
   */
  class TcsIO {
    private:
      const std::string name;   ///< name of this tcs
      const std::string host;   ///< hostname for this tcs
      int port;                 ///< port on hostname
      char term_write;          ///< terminate commands sent with this character
      char term_read;           ///< responses read are terminated with this character

      // connection for slow-response commands
      std::shared_ptr<Network::Interface> conn_slow;
      std::mutex mtx_slow;

      // pool for fast-response commands
      std::vector<std::shared_ptr<Network::Interface>> pool;
      std::mutex mtx_fast;
      std::condition_variable cv;

    public:
      TcsIO(const std::string &name, const std::string &host, int port, char term_write, char term_read)
        : name(name),
          host(host),
          port(port),
          term_write(term_write),
          term_read(term_read),
          conn_slow(std::make_shared<Network::Interface>(name, host, port, term_write, term_read))
      {
          for ( size_t i=0; i < poolsize; ++i ) {
            pool.emplace_back(std::make_shared<Network::Interface>(name, host, port, term_write, term_read));
          }
      }

      static const size_t poolsize = 3;

      // get a connection for a command
      //
      std::shared_ptr<Network::Interface> get_connection( bool is_slow_command ) {
        if ( is_slow_command ) {
          // slow command, lock and return the slow command connection
          std::lock_guard<std::mutex> lock( mtx_slow );
          return conn_slow;
        }
        else {
          // fast command, take a connection from the pool
          std::unique_lock<std::mutex> lock(mtx_fast);
          cv.wait( lock, [this]() { return !pool.empty(); } );
          auto conn = std::move( pool.back() );
          pool.pop_back();
          return conn;
        }
      }

      // return a fast-response connection to the pool
      //
      void return_connection( std::shared_ptr<Network::Interface> conn ) {
        {
        std::lock_guard<std::mutex> lock(mtx_fast);
        pool.push_back( std::move(conn) );
        }
        cv.notify_one();
      }

      /**
       * @var    tcs  pointer to Network::Interface
       * @brief  provides standard TCP/IP socket iterface
       */
      std::unique_ptr< Network::Interface > tcs;

      long execute_command(const std::string& command, std::string& response, bool is_slow_command) {
        auto conn = get_connection(is_slow_command);
        if (!conn) {
          logwrite("TCS::TcsIO::execute_command", "ERROR failed to acquire a connection");
          return ERROR;
        }

        long error = conn->send_command(command, response);
        if (!is_slow_command) {
          return_connection(conn); // Return fast connections to the pool
        }

      return error;
      }

      /**
       * convenience functions provide access to the Network::Interface functions
       */
      inline std::string device_name() const { return this->tcs->get_name(); }
      inline long open() const { return tcs->open(); }
      inline long close() const { return tcs->close(); }
      inline bool isconnected() const { return tcs->sock.isconnected(); }
      inline int fd() const { return tcs->sock.getfd(); }
      inline std::string gethost() const { return tcs->get_host(); }
      inline std::string getname() const { return tcs->get_name(); }
      inline int getport() const { return tcs->get_port(); }
      inline long send( std::string cmd ) { return tcs->send_command( cmd ); }
      inline long send( std::string cmd, std::string &retstring ) { return tcs->send_command( cmd, retstring ); }
  };
  /***** TCS::TcsIO ***********************************************************/


/****
  class ConnectionPool {
    public:
      std::queue<std::shared_ptr<TcsIO>> slow_connections;
      std::queue<std::shared_ptr<TcsIO>> fast_connections;

      std::mutex pool_mutex;

      ConnectionPool( size_t slow_count, size_t fast_count ) {
        for ( size_t i=0; i<slow_count; ++i ) slow_connections.push( std::make_shared<TcsIO>(true) );
        for ( size_t i=0; i<fast_count; ++i ) fast_connections.push( std::make_shared<TcsIO>(true) );
      }

      std::shared_ptr<TcsIO> get_connection( bool slow_command ) {
        std::lock_guard<std::mutex> lock(pool_mutex);

        auto &queue = slow_command ? slow_connections : fast_connections;

        if ( ! queue.empty() ) {
          auto conn = queue.front();
          queue.pop();
          return conn;
        }
        // return a new dynamic connection if pool is exhausted
        //
        return std::make_shared<TcsIO>();
      }

      void release_connection( bool slow_command, std::shared_ptr<TcsIO> conn ) {
        if ( conn->is_persistent ) {
          std::lock_guard<std::mutex> lock(pool_mutex);

          auto &queue = slow_command ? slow_connections : fast_connections;
          queue.push(conn);
        }
        // otherwise dynamic connections are destroyed when their shared ptr is released
      }
  };
*****/


  /***** TCS::Interface *******************************************************/
  /**
   * @class   Interface
   * @brief   interface class for a tcs device
   * @details This is the upper-level interface. The TcsIO class provides
   *          the lover-level socket I/O.
   *
   * This class defines the main interface for each tcs controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    public:
      int offsetrate_ra;                           ///< offset rate (arcsec/hr) for RA
      int offsetrate_dec;                          ///< offset rate (arcsec/hr) for DEC

      std::string name;                            ///< the name of the currently open tcs device

      std::map< std::string, std::shared_ptr<TCS::TcsIO> > tcsmap;  ///< STL map of TcsIO objects indexed by name

      std::mutex publish_mutex;
      std::mutex collect_mutex;

      std::mutex query_mtx;

      std::condition_variable publish_condition;
      std::condition_variable collect_condition;

      std::atomic<bool> publish_enable;
      std::atomic<bool> collect_enable;

      Interface() : offsetrate_ra(-1), offsetrate_dec(-1), publish_enable(false), collect_enable(false) { };

      void make_telemetry_message( std::string &retstring );  ///< assembles a telemetry message from tcs_info

      /**
       * These are the functions for communicating with the TCS
       */
      long list( const std::string &arg, std::string &retstring );
      long llist( const std::string &arg, std::string &retstring );
      long open( const std::string &arg, std::string &retstring );
      bool isopen();
      long isopen( std::string &retstring );
      long isopen( const std::string &arg, std::string &retstring );
      long close();
      long get_name( const std::string &arg, std::string &retstring );
      long get_weather_coords( const std::string &arg, std::string &retstring );
      long get_coords( const std::string &arg, std::string &retstring );
      long get_cass( const std::string &arg, std::string &retstring );
      long get_dome( const std::string &arg, std::string &retstring );
      long set_focus( const std::string &arg, std::string &retstring );
      long get_focus( std::string &retstring );
      long get_focus( const std::string &arg, std::string &retstring );
      long get_offsets( const std::string &arg, std::string &retstring );
      long get_offsets( double &raoff, double &decoff );
      long offsetrate( const std::string &arg, std::string &retstring );
      long get_motion( const std::string &arg, std::string &retstring );
      long ringgo( const std::string &arg, std::string &retstring );
      long coords( std::string args, std::string &retstring );
      long pt_offset( std::string args, std::string &retstring );
      long ret_offsets( std::string args, std::string &retstring );
      long send_command( std::string cmd, std::string &retstring, bool block=false );
      long native( std::string args, std::string &retstring );
      long parse_reply_code( std::string codein, std::string &retstring );
      long parse_motion_code( std::string codein, std::string &retstring );

      TcsInfo tcs_info;                            ///< contains information from the tcs
      long get_tcs_info();                         ///< fills the tcs_info class

      Common::Queue async;                         ///< asynchronous message queue object
  };
  /***** TCS::Interface *******************************************************/

}
/***** TCS ********************************************************************/
