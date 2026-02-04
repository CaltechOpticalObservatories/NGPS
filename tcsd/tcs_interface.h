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

  extern std::string IP_PALOMAR;                 ///< NGPS' IP address at Palomar
  extern std::string IP_TCS;                     ///< IP address of P200 TCS

  const std::string DAEMON_NAME = "tcsd";        ///< when run as a daemon, this is my name

  constexpr bool BLOCK=true;
  constexpr int  PTOFFSET_MIN_TIMEOUT=5000;      ///< minimum timeout [ms] for PT offset

  enum ConnectionType {
    FAST_RESPONSE,
    SLOW_RESPONSE
  };

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
      bool isopen;          /// is connection open to TCS
      std::string tcsname;  /// name of connected TCS { real sim }
      std::string utc;      /// ddd hh:mm:ss
      std::string lst;      /// hh:mm:ss
      std::string ha;       /// hh:mm:ss.s
      std::string ra_hms;   /// hh:mm:ss.ss
      std::string dec_dms;  /// dd:mm:ss.ss
      std::string motion;   /// one of the human readable TCS_MOTION*_STRs from tcs_constants.h

      double ra_h_dec;      /// h.hhhhh (decimal hours)
      double dec_d_dec;     /// d.ddddd (decimal degrees)
      double azimuth;
      double zenithangle;
      double domeazimuth;

      double airmass;
      double focus;
      double offsetra;
      double offsetdec;
      double offsetrate;
      double cassangle;
      double pa;

      int domeshutters;

      TcsInfo()
        : isopen(false) { this->init(); }

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
        offsetrate=NAN;
        cassangle=NAN;
        domeshutters=-1;
      }

      // These functions parse the return string from native TCS commands
      // and store the results in the class variables.
      //
      void parse_weather( std::string &input );  ///< parse retstring from native ?WEATHER
      void parse_reqstat( std::string &input );  ///< parse retstring from native REQSTAT
      void parse_reqpos( std::string &input );   ///< parse retstring from native REQPOS
      void parse_pa( std::string &input );       ///< parse retstring from native ?PARALLACTIC
  };
  /***** TCS::TcsInfo *********************************************************/


  /***** TCS::Connection ******************************************************/
  /**
   * @class   Connection
   * @brief   provides a connection for use in the TcsIO connection pool
   *
   */
  class Connection {
    public:
      std::shared_ptr<Network::Interface> socket;
      bool inuse;
      bool isopen;
      Connection(std::shared_ptr<Network::Interface> c) : socket(std::move(c)), inuse(false), isopen(false) {}
  };
  /***** TCS::Connection ******************************************************/


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

      // socket connection for slow-response commands
      std::shared_ptr<Network::Interface> sock_slow;
      std::mutex mtx_slow;
      std::atomic<bool> slow_inuse;

      std::mutex mtx_pool;
      std::condition_variable cv;

    public:
      TcsIO(const std::string &_name, const std::string &_host, int _port, char _term_write, char _term_read)
        : name(_name),
          host(_host),
          port(_port),
          term_write(_term_write),
          term_read(_term_read),
          sock_slow(std::make_shared<Network::Interface>(_name, _host, _port, _term_write, _term_read)),
          slow_inuse(false) { }

      static const size_t poolsize = 3;   ///< size of fast-response connection pool

      std::vector<Connection> pool;       ///< pool for fast-response commands

      std::string gethost() const { return this->host; }
      std::string getname() const { return this->name; }

      /***** TCS::TcsIO::initialize_sockets ***********************************/
      /**
       * @brief      create and open sockets to TCS
       * @details    This creates and opens poolsize+1 sockets, 1 for slow-response
       *             commands and poolsize for fast-response commands, which are
       *             placed into a pool.
       * @return     ERROR | NO_ERROR
       *
       */
      long initialize_sockets() {
        logwrite( "TCS::TcsIO::initialize_sockets", name+" host "+host+":"+std::to_string(port) );

        {
        std::lock_guard<std::mutex> lock( mtx_slow );
        if ( sock_slow->open() != NO_ERROR ) {
          logwrite( "TCS::TcsIO::initialize_sockets", "ERROR initializing slow-command socket connection" );
          return ERROR;
        }
        logwrite( "TCS::TcsIO::initialize_sockets", "initialized slow-command socket connection on fd "
                            +std::to_string(sock_slow->sock.getfd())+" at "+host+":"+std::to_string(port) );
        }

        // initialize pool of fast-response command sockets
        //
        {
        std::unique_lock<std::mutex> lock(mtx_pool);
        for ( size_t i=0; i<TcsIO::poolsize; ++i ) {
          auto sock = std::make_shared<Network::Interface>(name, host, port, term_write, term_read);
          if ( sock->open() != NO_ERROR ) {
            logwrite( "TCS::TcsIO::initialize_sockets", "ERROR initializing pool socket connection " + std::to_string(i) );
            return ERROR;
          }
          pool.push_back(Connection(sock));
          logwrite( "TCS::TcsIO::initialize_sockets", "initialized fast-command socket connection "
                    +std::to_string(i) + " on fd " + std::to_string(sock->sock.getfd())
                    +" at "+host+":"+std::to_string(port) );
        }
        }
        return NO_ERROR;
      }
      /***** TCS::TcsIO::initialize_sockets ***********************************/

      /***** TCS::TcsIO::get_connections **************************************/
      /**
       * @brief      return a shared pointer to a socket connection
       * @param[in]  conn_type  TCS::ConnectionType specifies slow or fast response
       * @return     shared_ptr<Network::Interface>
       *
       */
      std::shared_ptr<Network::Interface> get_connection( TCS::ConnectionType conn_type ) {
        const std::string function="TCS::TcsIO::get_connection";

        if ( conn_type == TCS::SLOW_RESPONSE ) {
          // slow command, lock and return the slow command socket connection
          std::lock_guard<std::mutex> lock( mtx_slow );
//        logwrite( function, "[DEBUG] slow command socket connection acquired on fd "
//                            +std::to_string(sock_slow->sock.getfd())+" for "+name+" at "+host+":"+std::to_string(port) );
          return sock_slow;
        }
        else {
          std::unique_lock<std::mutex> lock(mtx_pool);
          // fast command, take a socket connection from the pool
          while ( true ) {
            for ( auto &conn : pool ) {
//            logwrite(function, "[DEBUG] checking connection: fd " + std::to_string(conn.socket->sock.getfd()) +
//                               ", inuse: " + std::to_string(conn.inuse) +
//                               ", connected: " + std::to_string(conn.socket->sock.isconnected()));
              if ( !conn.socket->sock.isconnected() ) {
                logwrite( function, "fast command socket fd "+std::to_string(conn.socket->sock.getfd())
                                    +" not open, attempting to reconnect" );
                if ( conn.socket->open() != NO_ERROR ) {
                  logwrite( function, "ERROR opening fast command socket connection" );
                  return nullptr;
                }
//              logwrite( function, "[DEBUG] returning fast command socket connection on fd "
//                                  +std::to_string(conn.socket->sock.getfd()) );
                return conn.socket;
              }
              if ( !conn.inuse ) {
                conn.inuse=true;
//              logwrite( function, "[DEBUG] fast command socket connection acquired on fd "
//                                  +std::to_string(conn.socket->sock.getfd()) );
                return conn.socket;
              }
              logwrite( function, "fast command socket fd "+std::to_string(conn.socket->sock.getfd())+" inuse, trying another" );
            }
            // wait up to TIMEOUT ms for a connection if not immediately available
            std::cv_status status = cv.wait_for(lock, std::chrono::milliseconds(POLLTIMEOUT));
            if ( status == std::cv_status::timeout ) {
              logwrite( function, "timed out waiting for socket connection" );
              return nullptr;
            }
          }
        }
      }
      /***** TCS::TcsIO::get_connections **************************************/

      /***** TCS::TcsIO::execute_command **************************************/
      /**
       * @brief      get a connection and send a command to that connection
       * @param[in]  cmd        command to send to TCS
       * @param[in]  reply      reference to string holds reply
       * @param[in]  conn_type  TCS::ConnectionType specifies connection type
       * @return     ERROR | NO_ERROR
       *
       * This command is overloaded, calls execute_command with a default timeout.
       *
       */
      long execute_command( const std::string &cmd, std::string &reply, TCS::ConnectionType conn_type ) {
std::stringstream message;
message << "[DEBUG] in 3 arg version and using polltimeout=" << POLLTIMEOUT;
logwrite("TCS::TcsIO::execute_command", message.str());
        return execute_command( cmd, reply, conn_type, POLLTIMEOUT );
      }
      /***** TCS::TcsIO::execute_command **************************************/
      /**
       * @brief      get a connection and send a command to that connection
       * @param[in]  cmd        command to send to TCS
       * @param[in]  reply      reference to string holds reply
       * @param[in]  conn_type  TCS::ConnectionType specifies connection type
       * @param[in]  timeout    timeout for Poll in milliseconds
       * @return     ERROR | NO_ERROR
       *
       * This command is overloaded.
       *
       */
      long execute_command( const std::string &cmd, std::string &reply, TCS::ConnectionType conn_type, int timeout ) {
        const std::string function("TCS::TcsIO::execute_command");
std::stringstream message;
message << "[DEBUG] in 4 arg version with timeout=" << timeout;
logwrite(function,message.str());
        long ret=ERROR;

        if ( conn_type == TCS::SLOW_RESPONSE ) {
          // slow command
          {
          std::lock_guard<std::mutex> lock( mtx_slow );
          logwrite( function, "[DEBUG] slow command socket acquired on fd "
                              +std::to_string(sock_slow->sock.getfd())+" for "+name+" at "+host+":"+std::to_string(port)
                              +" timeout="+std::to_string(timeout) );
          ret = sock_slow->send_command( cmd, reply, timeout );
          }
          logwrite( function, "[DEBUG] releasing slow command socket connection on fd "
                              +std::to_string(sock_slow->sock.getfd())+" for "+name+" at "+host+":"+std::to_string(port) );
          if (ret!=NO_ERROR) sock_slow->reconnect();
          return ret;
        }
        else {
//        logwrite(function,"[DEBUG] asking for fast connection");
          auto conn = this->get_connection( TCS::FAST_RESPONSE );
          if (conn) {
//          logwrite(function,"[DEBUG] sending fast command");
            ret = conn->send_command( cmd, reply );
//          logwrite(function,"[DEBUG] fast command sent");
//          logwrite(function,"[DEBUG] returning fast connection");
            return_connection( conn );
          }
          else {
            logwrite( function, "failed to get fast connection" );
            return ERROR;
          }
          if (ret!=NO_ERROR) conn->reconnect();
          return ret;
        }
      }
      /***** TCS::TcsIO::execute_command **************************************/

      /***** TCS::TcsIO::return_connection ************************************/
      /**
       * @brief      return a connection to the pool
       * @details    Returns a fast-response socket to the pool for use by another
       *             thread.
       * @param[in]  sock  shared pointer to socket connection
       *
       */
      void return_connection( std::shared_ptr<Network::Interface> sock ) {
        const std::string function="TCS::TcsIO::return_connection";
        std::lock_guard<std::mutex> lock(mtx_pool);
        for ( auto &conn : pool ) {
          if ( conn.socket == sock ) {
            conn.inuse = false;
//          logwrite( function, "[DEBUG] returned socket connection to pool for fd "
//                              +std::to_string(conn.socket->sock.getfd()) );
            cv.notify_one();  // notifies any waiting get_connections()
            return;
          }
          logwrite( function, "socket connection not found for fd "+std::to_string(conn.socket->sock.getfd()) );
        }
      }
      /***** TCS::TcsIO::return_connection ************************************/

      /***** TCS::TcsIO::isconnected ******************************************/
      /**
       * @brief      returns true if any sockets are connected to the TCS
       * @return     true | false
       *
       */
      bool isconnected() {
        // Check if slow socket connection is connected, or if any connection in the pool is connected
        if (sock_slow && sock_slow->sock.isconnected()) {
          return true;
        }
        // Check fast connections in the pool
        std::lock_guard<std::mutex> lock(mtx_pool);
        for (const auto& conn : pool) {
          if (conn.socket->sock.isconnected()) {
            return true;
          }
        }
        return false;
      }
      /***** TCS::TcsIO::isconnected ******************************************/

      /***** TCS::TcsIO::close ************************************************/
      /**
       * @brief      closes all socket connections
       * @return     ERROR | NO_ERROR
       *
       */
      long close() {
        long error=NO_ERROR;
        if (sock_slow && sock_slow->sock.isconnected()) {
          logwrite( "TCS::TcsIO::close", "closing slow-command socket fd "+std::to_string(sock_slow->sock.getfd()) );
          error |= sock_slow->close();
        }
        // Check fast connections in the pool
        std::lock_guard<std::mutex> lock(mtx_pool);
        for (const auto& conn : pool) {
          if (conn.socket->sock.isconnected()) {
            logwrite( "TCS::TcsIO::close", "closing fast-command socket fd "+std::to_string(conn.socket->sock.getfd()) );
            error |= conn.socket->close();
          }
        }
        return error;
      }
      /***** TCS::TcsIO::close ************************************************/
  };
  /***** TCS::TcsIO ***********************************************************/


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
    private:
      zmqpp::context context;
      std::string default_tcs;                     ///< default TCS to use specified in .cfg

    public:
      inline void set_default_tcs(const std::string &which) { this->default_tcs=which; }

      int offsetrate;                              ///< offset rate (arcsec/hr)

      std::string name;                            ///< the name of the currently open tcs device

      std::map< std::string, std::shared_ptr<TCS::TcsIO> > tcsmap;  ///< STL map of TcsIO objects indexed by name

      std::mutex publish_mutex;
      std::mutex collect_mutex;

      std::mutex query_mtx;

      std::condition_variable publish_condition;
      std::condition_variable collect_condition;

      std::atomic<bool> publish_enable;
      std::atomic<bool> collect_enable;

      Interface()
        : context(),
          offsetrate(0),
          publish_enable(false),
          collect_enable(false),
          subscriber(std::make_unique<Common::PubSub>(context, Common::PubSub::Mode::SUB)),
          is_subscriber_thread_running(false),
          should_subscriber_thread_run(false)
      {
        topic_handlers = {
              { "_snapshot", std::function<void(const nlohmann::json&)>(
                         [this](const nlohmann::json &msg) { handletopic_snapshot(msg); } ) }
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

      // publish/subscribe functions
      //
      long init_pubsub(const std::initializer_list<std::string> &topics={}) {
        return Common::PubSubHandler::init_pubsub(context, *this, topics);
      }
      void start_subscriber_thread() { Common::PubSubHandler::start_subscriber_thread(*this); }
      void stop_subscriber_thread()  { Common::PubSubHandler::stop_subscriber_thread(*this); }

      void handletopic_snapshot( const nlohmann::json &jmessage );

      void publish_snapshot();
      void publish_snapshot(std::string &retstring);

      /**
       * These are the functions for communicating with the TCS
       */
      long list( const std::string &arg, std::string &retstring );
      long llist( const std::string &arg, std::string &retstring );
      long open( std::string arg, std::string &retstring );
      long open();
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
      long get_pa(const std::string &arg, std::string &retstring);
      long pt_offsetrate( const std::string &arg, std::string &retstring );
      long get_motion( const std::string &arg, std::string &retstring );
      long ringgo( const std::string &arg, std::string &retstring );
      long coords( std::string args, std::string &retstring );
      long pt_offset( std::string args, std::string &retstring );
      long zero_offsets( const std::string args, std::string &retstring );
      long ret_offsets( std::string args, std::string &retstring );
      long send_command( std::string cmd, std::string &retstring, TCS::ConnectionType conn_type );
      long send_command( std::string cmd, std::string &retstring, TCS::ConnectionType conn_type, int to );
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
