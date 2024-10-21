/**
 * @file     common.h
 * @brief    Common namespace header file
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <sstream>
#include <queue>
#include <string>
#include <mutex>
#include <map>
#include <condition_variable>
#include <regex>
#include <json.hpp>

#include "logentry.h"
#include "network.h"

const long NOTHING = -1;
const long NO_ERROR = 0;
const long ERROR = 1;
const long BUSY = 2;
const long TIMEOUT = 3;
const long HELP = 4;
const long JSON = 5;
const long EXIT = 999;

const std::string JEOF = "EOF\n";              ///< used to terminate JSON messages
const std::string TELEMREQUEST = "sendtelem";  ///< common daemon command used to request telemetry

constexpr bool EXT = true;   ///< constant for use_extension arg of Common::Header::add_key()
constexpr bool PRI = !EXT;   ///< constant for use_extension arg of Common::Header::add_key()


/***** Common *****************************************************************/
/**
 * @namespace Common
 * @brief     common namespace provides classes that might be used by various modules
 *
 */
namespace Common {

  /**************** Common::FitsKeys ******************************************/
  /*
   * @class  FitsKeys
   * @brief  provides a user-defined keyword database
   *
   */
  class FitsKeys {
    private:
    public:
      FitsKeys() {}
      ~FitsKeys() {}

      std::string get_keytype(std::string keyvalue);         ///< return type of keyword based on value
      long listkeys();                                       ///< list FITS keys in the internal database
      long delkey( const std::string &key );                 ///< delete FITS key from the internal database
      long addkey( const std::string &arg );                 ///< add FITS key to the internal database
      long addkey( const std::vector<std::string> &vec );    ///< add FITS key to the internal database

      /***** Common::FitsKeys::addkey *****************************************/
      /**
       * @brief      template function adds FITS keyword to internal database
       * @details    no parsing is done here
       * @param[in]  keyword  keyword name <string>
       * @param[in]  value    value for keyword, any type <T>
       * @param[in]  comment  comment <string>
       * @param[in]  prec     decimal places of precision
       * @return     ERROR or NO_ERROR
       *
       * This function is overloaded.
       *
       * This version adds the keyword=value//comment directly into the
       * database with no parsing. The database stores only strings so
       * the type variable preserves the type and informs the FITS writer.
       *
       * The template input "tval" here allows passing in any type of value,
       * and the type will be set by the actual input type. However, if
       * the input type is a string then an attempt will be made to infer the
       * type from the string (this allows passing in a string "12.345" and
       * have it be converted to a double.
       *
       * The prec argument sets the displayed precision of double or float
       * types if passed in as a double or float. Default is 8 if not supplied.
       *
       * The overloading for type bool is because for the template class,
       * since T can be evaluated as string, it can't be treated as bool,
       * so this is the easiest way to extract the bool value.
       *
       */
      inline long addkey( const std::string &key, bool bval, const std::string &comment ) {
        return addkey( key, (bval?"T":"F"), comment, 0 );
      }
      template <class T> long addkey( const std::string &key, T tval, const std::string &comment ) {
        return addkey( key, tval, comment, 8 );
      }
      template <class T> long addkey( const std::string &key, T tval, const std::string &comment, int prec ) {
        std::stringstream val;
        std::string type;

        // "convert" the type <T> value into a string via the appropriate stream
        // and set the type string.
        //
        // The use of constexpr() ensures that the compiler discards brances
        // that are not valid for the given T.
        //
        if constexpr( std::is_same<T, double>::value ) {
          val << std::fixed << std::setprecision( prec );
          if ( !std::isnan(tval) ) val << tval; else val << "NaN";
          type = ( !std::isnan(tval) ? "DOUBLE" : "STRING" );
        }
        else
        if constexpr( std::is_same<T, float>::value ) {
          val << std::fixed << std::setprecision( prec );
          if ( !std::isnan(tval) ) val << tval; else val << "NaN";
          type = ( !std::isnan(tval) ? "FLOAT" : "STRING" );
        }
        else
        if constexpr( std::is_same<T, int>::value ) {
          val << tval;
          type = "INT";
        }
        else
        if constexpr( std::is_same<T, long>::value ) {
          val << tval;
          type = "LONG";
        }
        else {
          val << tval;
          type = this->get_keytype( val.str() );  // try to infer the type from any string
        }

        if ( key.empty() || ( key.find(" ") != std::string::npos ) ) return NO_ERROR;

        // insert new entry into the database
        //
        this->keydb[key].keyword    = key;
        this->keydb[key].keytype    = type;
        this->keydb[key].keyvalue   = val.str();
        this->keydb[key].keycomment = comment;

//#ifdef LOGLEVEL_DEBUG
//        std::string function = "Common::FitsKeys::addkey";
//        std::stringstream message;
//        message << "[DEBUG] added key type " << type << ": " << key << "=" << tval << " (" << this->keydb[key].keytype << ") // " << comment;
//        logwrite( function, message.str() );
//#endif
        return( NO_ERROR );
      }
      /***** Common::FitsKeys::addkey *****************************************/

      /**
       * @typedef user_key_t
       * @brief   structure of FITS keyword internal database
       */
      typedef struct {
        std::string keyword;
        std::string keytype;
        std::string keyvalue;
        std::string keycomment;
      } user_key_t;

      /**
       * @typedef fits_key_t
       * @brief   STL map for the actual keyword database
       */
      typedef std::map<std::string, user_key_t> fits_key_t;

      fits_key_t keydb;                                      ///< keyword database

      inline void erase_db() { this->keydb.clear(); return; }

void merge( Common::FitsKeys from ) {
  this->keydb.insert( from.keydb.begin(), from.keydb.end() );
  return;
}

      // Find all entries in the keyword database which start with the search_for string,
      // return a vector of iterators.
      //
      std::vector< fits_key_t::const_iterator > FindKeys( std::string search_for ) {
        std::vector< fits_key_t::const_iterator > vec;
        for ( auto it  = this->keydb.lower_bound( search_for );
                   it != std::end( this->keydb ) && it->first.compare( 0, search_for.size(), search_for ) == 0;
                 ++it ) {
          vec.push_back( it );
        }
        return( vec );
      }

      // Find and remove all entries in the keyword database which match the search_for string.
      //
      void EraseKeys( std::string search_for ) {
        std::string function = "FitsKeys::EraseKeys";
        std::stringstream message;
        std::vector< fits_key_t::const_iterator > erasevec;

        // get a vector of iterators for all the keys matching the search string
        //
        erasevec = this->FindKeys( search_for );
//#ifdef LOGLEVEL_DEBUG
//        message.str(""); message << "[DEBUG] found " << erasevec.size() << " entries matching \"" << search_for << "*\"";
//        logwrite( function, message.str() );
//#endif

        // loop through that vector and erase the selected iterators from the database
        //
        for ( auto vec : erasevec ) {
//#ifdef LOGLEVEL_DEBUG
//          message.str(""); message << "[DEBUG] erasing " << vec->first; logwrite( function, message.str() );
//#endif
          this->keydb.erase( vec );
        }
        return;
      }
  };
  /**************** Common::FitsKeys ******************************************/


  /**************** Common::Header ********************************************/
  /*
   * @class   Header
   * @brief   encapsulates FitsKeys classes to hold primary & extension databases
   * @details contains template functions to add key,value,comment passed by
   *          reference or from a JSON message
   *
   */
  using json = nlohmann::json;
  class Header {
    private:
      FitsKeys _primary;
      FitsKeys _extension;

    public:
      FitsKeys &primary()   { return _primary;   }
      FitsKeys &extension() { return _extension; }

      /**
       * @brief      template class adds key,value,comment to indicated keydb
       * @param[in]  keydb    reference to FitsKeys database map
       * @param[in]  keyword  name of header keyword
       * @param[in]  value    value of template type <T>
       * @param[in]  comment  comment string for header keyword
       *
       */
      template <typename T>
      void add_key( FitsKeys &keydb,
                    const std::string &keyword, const T &value, const std::string &comment ) {
        keydb.addkey( keyword, value, comment );
      }

      /**
       * @brief      template class adds key,value,comment to indicated keydb from json message
       * @details    This extracts the value from a JSON message and uses add_key to add the
       *             keyword to the indicated database map.
       * @param[in]  type      reference to FitsKeys database object
       * @param[in]  jmessage  JSON message is the source of the value
       * @param[in]  comment   comment string for header keyword
       *
       */
      template <typename HeaderType, typename T=nlohmann::json>
      void add_json_key( HeaderType type,
                         const T &jmessage,
                         const std::string &keyword, const std::string &comment ) {
        const std::string function="Common::Header::add_json_key";
        std::stringstream message;

        try {
          // extract the value from the JSON message using keyword as the key
          //
          auto jvalue = jmessage.at( keyword );

          // use this object's db type
          //
          FitsKeys &keydb = (this->*type)();

          // type check each value to call add_key with the correct type
          //
          if ( jvalue.type() == json::value_t::boolean ) {
            this->add_key( keydb, keyword, jvalue.template get<bool>(), comment );
          }
          else
          if ( jvalue.type() == json::value_t::number_integer ) {
            this->add_key( keydb, keyword, jvalue.template get<int>(), comment );
          }
          else
          if ( jvalue.type() == json::value_t::number_unsigned ) {
            this->add_key( keydb, keyword, jvalue.template get<uint16_t>(), comment );
          }
          else
          if ( jvalue.type() == json::value_t::number_float ) {
            this->add_key( keydb, keyword, jvalue.template get<double>(), comment );
          }
          else
          if ( jvalue.type() == json::value_t::string ) {
            this->add_key( keydb, keyword, jvalue.template get<std::string>(), comment );
          }
          else {
            message << "ERROR unknown type for keyword " << keyword << "=" << jvalue;
            logwrite( function, message.str() );
          }
        }
        catch( const json::exception &e ) {
          message << "JSON exception adding keyword " << keyword << ": " << e.what();
          logwrite( function, message.str() );
        }
        catch( const std::exception &e ) {
          message << "ERROR exception adding keyword " << keyword << ": " << e.what();
          logwrite( function, message.str() );
        }
      }
  };
  /**************** Common::Header ********************************************/


  /**************** Common::Queue *********************************************/
  /**
   * @class  Queue
   * @brief  provides a thread-safe messaging queue
   *
   */
  class Queue {
    private:
      std::queue<std::string> message_queue;
      mutable std::mutex queue_mutex;
      std::condition_variable notifier;
      bool is_running;
    public:
      Queue(void) : message_queue() , queue_mutex() , notifier() { this->is_running = false; };
      ~Queue(void) {}

      void service_running(bool state) { this->is_running = state; };  ///< set service running
      bool service_running() { return this->is_running; };             ///< is the service running?

      void enqueue_and_log(std::string function, std::string message);
      void enqueue_and_log(std::string tag, std::string function, std::string message);
      void enqueue(std::string message);                               ///< push an element into the queue.
      std::string dequeue(void);                                       ///< pop an element from the queue
  };
  /**************** Common::Queue *********************************************/


  /***** Common::DaemonClient *********************************************************/
  /**
   * @class   DaemonClient
   * @brief   defines a daemon-object for communicating with a daemon
   * @details This instantiates a Network::TcpSocket object for performing the
   *          actual socket communication.
   *
   */
  class DaemonClient {
    private:
      std::mutex client_access;
      char term_write;            ///< send adds this char on Writes
      char term_read;             ///< send looks for this char on Reads (if reply requested)
      std::string term_str_write; ///< optional terminating string for writes
      std::string term_str_read;  ///< optional terminating string for reads
      bool term_with_string;      ///< set true if using termating string (else terminating char)
      bool timedout;
      std::atomic<int> num_send;

    public:
      DaemonClient()
        : term_write('\n'), term_read('\n'), term_with_string(false),
          timedout(false), num_send(2), name("unconfigured_daemon"), port(-1), nbport(-1) {
        this->socket.sethost( "localhost" );
      };

      DaemonClient( std::string name_in )
        : term_write('\n'), term_read('\n'), term_with_string(false),
          timedout(false), num_send(2), name(name_in), port(-1), nbport(-1) {
        this->socket.sethost( "localhost" );
      };   ///< preferred constructor with name to identify daemon

      DaemonClient( std::string name_in, char write_in, char read_in )
        : term_write(write_in), term_read(read_in), term_with_string(false),
          timedout(false), num_send(2), name(name_in), port(-1), nbport(-1) {
        this->socket.sethost( "localhost" );
      };   ///< preferred constructor with name to identify daemon

      DaemonClient( std::string name_in, const std::string &write_in, const std::string &read_in )
        : term_str_write(write_in), term_str_read(read_in), term_with_string(true),
          timedout(false), num_send(2), name(name_in), port(-1), nbport(-1) {
        this->socket.sethost( "localhost" );
      };   ///< construct with terminating strings

      std::string name;             ///< name of the daemon
      std::string host;             ///< host where the daemon is running
      int port;                     ///< blocking port that the daemon is listening on
      int nbport;                   ///< non-blocking port that the daemon is listening on

      Network::TcpSocket socket;    ///< socket object for communications with daemon

      long async( std::string args );                              ///< async (non-blocking) commands to daemon that don't need a reply
      long async( std::string args, std::string &retstring );      ///< async (non-blocking) commands to daemon that need a reply

      static void dothread_command( Common::DaemonClient &daemon, std::string args );
      long command( std::string args );                            ///< commands to daemon
      long command( std::string args, std::string &retstring );    ///< commands to daemon that need a reply
      long send( std::string command, std::string &reply );        ///< for internal use only
      long connect();                                              ///< initialize socket connection to daemon
      void disconnect();                                           ///< close socket connection to daemon
      inline void set_name( const std::string &name_in ) { this->name=name_in; } ///< name this daemon
      inline void set_port( const int &port ) { this->port=port; }               ///< set the port number

      long is_connected( std::string &reply );
      bool poll_open();                                            ///< is device open?
      bool is_open();                                              ///< is device open?
      bool is_open(bool silent);                                   ///< is device open?
  };
  /***** Common::DaemonClient *********************************************************/

}
/***** Common *****************************************************************/
