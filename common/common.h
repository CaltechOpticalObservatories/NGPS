/**
 * @file     common.h
 * @brief    Common namespace header file
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <climits>
#include <sstream>
#include <queue>
#include <string>
#include <mutex>
#include <map>
#include <condition_variable>
#include <regex>
#include <json.hpp>
#include <zmqpp/zmqpp.hpp>
#include <thread>
#include <iostream>

#include "logentry.h"
#include "network.h"

const long NOTHING = -1;
const long NO_ERROR = 0;
const long ERROR = 1;
const long BUSY = 2;
const long TIMEOUT = 3;
const long HELP = 4;
const long JSON = 5;
const long ABORT = 6;
const long EXIT = 999;

const std::string JEOF = "EOF\n";              ///< used to terminate JSON messages
const std::string TELEMREQUEST = "sendtelem";  ///< common daemon command used to request telemetry
const std::string SNAPSHOT = "snapshot";       ///< common daemon command forces publish of telemetry

constexpr bool EXT = true;   ///< constant for use_extension arg of Common::Header::add_key()
constexpr bool PRI = !EXT;   ///< constant for use_extension arg of Common::Header::add_key()

/***** Common *****************************************************************/
/**
 * @namespace Common
 * @brief     common namespace provides classes that might be used by various modules
 *
 */
namespace Common {

  using json = nlohmann::json;

  /**************** Common::PubSub ********************************************/
  /*
   * @class  PubSub
   * @brief  provides a publish/subscribe interface using ZeroMQ
   *
   */
  class PubSub {
    public:
      enum class Mode { PUB, SUB };

    private:
      zmqpp::context &_context;
      zmqpp::socket _socket;
      Mode _mode;                        ///< publisher or subscriber?
      std::string _topic;                ///< publisher topic
      std::vector<std::string> _topics;  ///< list of subscriber topics

    public:
      /**
       * @brief    generic constructor sets up the socket and mode
       */
      PubSub( zmqpp::context &context, Mode mode )
        : _context(context),
          _socket(context, (mode==Mode::PUB ? zmqpp::socket_type::publish : zmqpp::socket_type::subscribe)),
          _mode(mode) { }

      ~PubSub() { _socket.close(); }

      /**
       * @brief       publishers bind to a socket endpoint (not for brokers)
       */
      bool has_message() {
        zmqpp::poller poller;
        poller.add(_socket, zmqpp::poller::poll_in);
        return ( poller.poll(100) > 0 );
      }

      /**
       * @brief       publishers bind to a socket endpoint (not for brokers)
       * @param[in]   addr   broker endpoint
       * @param[in]   topic  default topic that publisher will use
       */
      void bind( const std::string &addr, const std::string &topic ) {
        if ( _mode != Mode::PUB ) {
          throw std::runtime_error( "(Common::PubSub::bind) not a publisher" );
        }
        _socket.bind( addr );
        _topic = topic;
      }

      /**
       * @brief       publishers will use connect when publishing to a broker
       * @param[in]   addr   broker endpoint
       * @param[in]   topic  default topic that publisher will use
       */
      void connect_to_broker( const std::string &addr, const std::string &topic ) {
        if ( _mode != Mode::PUB ) {
          throw std::runtime_error( "(Common::PubSub::connect_to_broker) not a publisher" );
        }
        _socket.connect( addr );
        _topic = topic;
      }

      /**
       * @brief       subscribers connect to publishers
       * @param[in]   addr   publisher's endpoint
       */
      void connect( const std::string &addr ) {
        if ( _mode != Mode::SUB ) {
          throw std::runtime_error( "(Common::PubSub::connect) not a subscriber" );
        }
        _socket.connect( addr );
      }

      /**
       * @brief       subscribe to a topic. can be called more than once
       * @param[in]   topic  topic to subscribe to
       */
      void subscribe( const std::string &topic ) {
        if ( _mode != Mode::SUB ) {
          throw std::runtime_error( "(Common::PubSub::subscribe) not a subscriber" );
        }
        _topics.push_back(topic);
        _socket.subscribe(topic);
      }

      /**
       * @brief       unsubscribe from a topic. can be called more than once
       * @param[in]   topic  topic to unsubscribe from
       */
      void unsubscribe( const std::string &topic ) {
        if ( _mode != Mode::SUB ) {
          throw std::runtime_error( "(Common::PubSub::unsubscribe) not a subscriber" );
        }
        auto it = std::find( _topics.begin(), _topics.end(), topic );
        if ( it != _topics.end() ) {
          _topics.erase(it);
          _socket.unsubscribe(topic);
        }
      }

      /**
       * @brief       receive a message from a publisher
       * @return      topic,payload
       */
      std::pair<std::string,std::string> receive() {
        if ( _mode != Mode::SUB ) {
          throw std::runtime_error( "(Common::PubSub::receive) not a subscriber" );
        }
        zmqpp::message message_zmq;
        _socket.receive( message_zmq );
        std::string topic, payload;
        message_zmq >> topic >> payload;
        return { topic, payload };
      }

      /**
       * @brief      publish
       * @param[in]  message  reference to JSON message
       * @param[in]  topic    publishing topic
       *
       */
      void publish( const json &message_out, const std::string &topic="" ) {
        if ( _mode != Mode::PUB ) {
          throw std::runtime_error( "(Common::PubSub::publish) not a publisher" );
        }
        zmqpp::message message_zmq;
        // Publish to either class default _topic or topic specified as
        // optional arg.
        message_zmq.add( topic.empty() ? _topic : topic );
        message_zmq.add( message_out.dump() );
        _socket.send( message_zmq );
      }
      /***** Common::PubSub::publish ******************************************/

  };
  /**************** Common::PubSub ********************************************/


  class PubSubHandler {
    public:
      template <typename Interface>
      /***** Common::PubSubHandler::init_pubsub *******************************/
      /**
       * @brief      initialize publisher, subscriber, and start subscriber thread
       * @return     ERROR | NO_ERROR
       *
       */
      static long init_pubsub( zmqpp::context &context, Interface &iface,
                               const std::initializer_list<std::string> &topics={} ) {
        const std::string function("Common::PubSubHandler::init_pubsub");

        // might not be right but you need something here
        if ( iface.subscriber_address.empty() ) {
          logwrite( function, "ERROR subscriber address not defined" );
          return ERROR;
        }

        // Initialize the message subscriber. All daemons minimally subscribe to "_snapshot"
        // which causes them to publish their current status.
        //
        iface.subscriber_topics.clear();
        iface.subscriber_topics.push_back("_snapshot");

        for ( const auto &topic : topics ) {
          iface.subscriber_topics.push_back(topic);
        }

        try {
          // connect to the message broker and wait for connection to establish
          //
          iface.subscriber->connect( iface.subscriber_address );
          std::this_thread::sleep_for( std::chrono::milliseconds(500) );

          // subscribe to configured topic(s)
          //
          for ( const auto &topic : iface.subscriber_topics ) {
            iface.subscriber->subscribe( topic );
            logwrite( function, "subscribed to topic: "+topic );
          }

          // connect publisher to the message broker and register my default topic
          //
          logwrite( function, "binding publisher to "+iface.publisher_address
                             +" with default topic " +iface.publisher_topic );
          iface.publisher = std::make_unique<Common::PubSub>( context, Common::PubSub::Mode::PUB );
          iface.publisher->connect_to_broker( iface.publisher_address, iface.publisher_topic );
        }
        catch ( const zmqpp::zmq_internal_exception &e ) {
          logwrite( function, "ERROR initializing message handler: "+std::string(e.what()) );
          return ERROR;
        }
        catch ( const std::runtime_error &e ) {
          logwrite( function, "ERROR initializing message handler: "+std::string(e.what()) );
          return ERROR;
        }
        catch ( const std::exception &e ) {
          logwrite( function, "ERROR initializing message handler: "+std::string(e.what()) );
          return ERROR;
        }

        // start the subscriber thread now
        //
        iface.start_subscriber_thread();

        return NO_ERROR;
      }
      /***** Common::PubSubHandler::init_pubsub *******************************/


      /***** Common::PubSubHandler::start_subscriber_thread *******************/
      /**
       * @brief      starts subscriber thread if needed
       *
       */
      template <typename Interface>
      static void start_subscriber_thread(Interface &iface) {
        iface.should_subscriber_thread_run.store(true);
        if ( !iface.is_subscriber_thread_running.load() ) {
          std::thread( &PubSubHandler::subscriber_thread<Interface>, std::ref(iface) ).detach();
        }
        else logwrite( "Common::PubSubHandler::start_subscriber_thread", "already running" );
      }
      /***** Common::PubSubHandler::start_subscriber_thread *******************/


      /***** Common::PubSubHandler::subscriber_thread *************************/
      /**
       * @brief      the actual subscriber thread
       *
       */
      template <typename Interface>
      static void subscriber_thread(Interface &iface) {
        logwrite( "Common::PubSubHandler::subscriber_thread", "subscriber started" );

        BoolState thread_running( iface.is_subscriber_thread_running );

        // listen for published messages and handle them
        //
        while ( iface.should_subscriber_thread_run ) {
          try {
            if ( iface.subscriber->has_message() ) {
              auto [topic,payload] = iface.subscriber->receive();
              process_incoming_message(iface, topic, payload);
            }
          }
          catch ( const std::exception &e ) {
            logwrite( "Common::PubSubHandler::subscriber_thread", "ERROR "+std::string(e.what()) );
            continue;
          }
        }
        logwrite( "Common::PubSubHandler::subscriber_thread", "subscriber terminated" );
      }
      /***** Common::PubSubHandler::subscriber_thread *************************/


      /***** Common::PubSubHandler::stop_subscriber_thread ********************/
      /**
       * @brief      stop the subscriber thread
       *
       */
      template <typename Interface>
      static void stop_subscriber_thread(Interface &iface) {
        iface.should_subscriber_thread_run.store(false);
        while ( iface.is_subscriber_thread_running.load() );
      }
      /***** Common::PubSubHandler::stop_subscriber_thread ********************/


      /***** Common::PubSubHandler::process_incomming_message *****************/
      /**
       * @brief      processes incoming subscribed telemetry messages
       * @param[in]  message_in  incoming message, expects serialized JSON string
       * @return     ERROR | NO_ERROR
       *
       */
      template <typename Interface>
      static long process_incoming_message( Interface &iface, std::string topic, std::string message_in ) {
        const std::string function="Common::PubSubHandler::process_incoming_message";

        // nothing to do if the message is empty
        //
        if ( message_in.empty() ) {
          logwrite( function, "ERROR empty message" );
          throw;
        }

        try {
          nlohmann::json jmessage = nlohmann::json::parse( message_in );

          // find this topic in the map of topic handlers
          //
          auto it = iface.topic_handlers.find( topic );

          // if this topic has a mapped handling function, then call it,
          // passing it the json message
          //
          if ( it != iface.topic_handlers.end() ) it->second( jmessage );
        }
        catch ( const nlohmann::json::parse_error &e ) {
          logwrite( function, "ERROR json exception parsing message topic "+topic+": "+std::string(e.what()) );
          logwrite( function, message_in );
          throw;
        }
        catch ( const std::exception &e ) {
          logwrite( function, "ERROR parsing message topic "+topic+": "+std::string(e.what()) );
          logwrite( function, message_in );
          throw;
        }

        return NO_ERROR;
      }
      /***** Common::PubSubHandler::process_incomming_message *****************/

  };


  void collect_telemetry(const std::pair<std::string,int> &provider, std::string &retstring);

  /***** Common::extract_telemetry_value **************************************/
  /**
   * @brief      extract a correctly typed value from a JSON message using a specific key
   * @param[in]  jstring  serialized JSON message string
   * @param[in]  jkey     key to look for in JSON string
   * @param[out] value    reference of type T to value to return
   *
   */
  template <typename T>
  void extract_telemetry_value( const std::string &jstring, const std::string &jkey, T &value ) {
    try {
      // get a JSON message object from the serialized string
      //
      json jmessage = json::parse( jstring );
      extract_telemetry_value( jmessage, jkey, value );
    }
    catch( const json::exception &e ) {
      logwrite( "Common::extract_telemetry_value", "ERROR JSON exception: "+std::string(e.what()) );
      return;
    }
  }
  template <typename T>
  void extract_telemetry_value( const nlohmann::json &jmessage, const std::string &jkey, T &value ) {
    const std::string function="Common::extract_telemetry_value";
    std::stringstream message;

    // extract the correct typed value for the requested key from the provided
    // serialized json string
    //
    try {
      // extract the value from the JSON message using jkey as the key
      //
      auto jvalue = jmessage.at( jkey );

      if ( jvalue.is_null() ) return;

      // Using the type of the supplied value reference T, try to extract
      // the corresponding type of data from the json key and assign it to
      // the provided value.
      //
      if constexpr ( std::is_same<T, bool>::value ) {
        if ( jvalue.type() == json::value_t::boolean ) {
          value = jvalue.template get<bool>();
        }
      }
      else
      if constexpr ( std::is_same<T, uint16_t>::value || std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value) {
        if ( jvalue.type() == json::value_t::number_unsigned ) {
          value = jvalue.template get<T>();
        }
      }
      else
      if constexpr ( std::is_same<T, int16_t>::value || std::is_same<T, int32_t>::value || std::is_same<T, int64_t>::value ) {
        if ( jvalue.type() == json::value_t::number_unsigned ) {
          value = jvalue.template get<T>();
        }
      }
      else
      if constexpr ( std::is_same<T, float>::value || std::is_same<T, double>::value ) {
        if ( jvalue.type() == json::value_t::number_float ) {
          value = jvalue.template get<T>();
        }
      }
      else
      if constexpr ( std::is_same<T, std::string>::value ) {
        if ( jvalue.type() == json::value_t::string ) {
          value = jvalue.template get<std::string>();
        }
      }
      else
      if constexpr ( std::is_same<T, std::vector<uint8_t>>::value ) {
        if ( jvalue.type() == json::value_t::binary ) {
          value = jvalue.template get<std::vector<uint8_t>>();
        }
      }
      else
      if constexpr ( std::is_same<T, std::vector<json>>::value ) {
        if ( jvalue.type() == json::value_t::array ) {
          value = jvalue.template get<std::vector<json>>();
        }
      }
      else
      if constexpr ( std::is_same<T, json>::value ) {
        if ( jvalue.type() == json::value_t::object ) {
          value = jvalue.template get<json>();
        }
      }
      else {
        message << "ERROR unknown type for key " << jkey;
        logwrite( function, message.str() );
        return;
      }
    }
    catch( const json::exception &e ) {
      message << "ERROR JSON exception parsing value for key " << jkey << ": " << e.what();
      logwrite( function, message.str() );
    }
    catch( const std::exception &e ) {
      message << "ERROR exception parsing value for key " << jkey << ": " << e.what();
      logwrite( function, message.str() );
    }
    return;
  }
  /***** Common::extract_telemetry_value **************************************/


  /**************** Common::FitsKeys ******************************************/
  /*
   * @class  FitsKeys
   * @brief  provides a user-defined keyword database
   *
   */
  class FitsKeys {
    public:
      /**
       * @brief   structure of FITS keyword internal database
       */
      typedef struct {
        std::string keyword;
        std::string keytype;
        std::string keyvalue;
        std::string keycomment;
      } struct_key_t;

      /**
       * @brief   STL map for the actual keyword database
       */
      typedef std::map<std::string, struct_key_t> fits_key_t;

      fits_key_t keydb;                                      ///< keyword database

      /**
       * @brief  default constructor
       */
      FitsKeys() { }

      /**
       * @brief  copy constructor, copies the keydb map
       */
      FitsKeys( const FitsKeys &other ) : keydb(other.keydb) { }

      /**
       * @brief  assignment operator, assigns the keydb map
       */
      FitsKeys &operator=(const FitsKeys &other) {
        if ( this != &other ) keydb = other.keydb;  ///< assign the map if not myself
        return *this;
      }

      /**
       * @brief  merges another fits_key_t database (keydb) with this one
       */
      void merge( const fits_key_t &source ) {
        for ( const auto &pair : source ) {
          this->keydb.insert(pair);
        }
      }

      /**
       * @brief  erases this fits_key_t database (keydb)
       */
      inline void erase_db() { this->keydb.clear(); return; }

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

#ifdef LOGLEVEL_DEBUG
        std::string function = "Common::FitsKeys::addkey";
        std::stringstream message;
        message << "[DEBUG] added key type " << type << ": " << key << "=" << tval << " (" << this->keydb[key].keytype << ") // " << comment;
        logwrite( function, message.str() );
#endif
        return( NO_ERROR );
      }
      /***** Common::FitsKeys::addkey *****************************************/

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
   * @details The Common::Header class contains two Common::FitsKeys objects
   *          contains template functions to add key,value,comment passed by
   *          reference or from a JSON message
   *
   */
  using json = nlohmann::json;
  class Header {
    private:
      FitsKeys _primary;
      FitsKeys _extension;
      std::map<std::string, FitsKeys> _elmo;

    public:
      Header() = default;

      /**
       * @brief  copy constructor
       */
      Header(const Header &other)
        : _primary(other._primary),
          _extension(other._extension),
          _elmo(other._elmo) { }

      /**
       * @brief  assignment operator
       */
      Header &operator=(const Header &other) {
        if (this != &other) {
          _primary = other._primary;
          _extension = other._extension;
          _elmo = other._elmo;
        }
        return *this;
      }

      /**
       * getter functions return references to the private FitsKeys
       * data objects
       */
      FitsKeys &primary()   { return _primary;   }  ///< return a reference to _primary object
      FitsKeys &extension() { return _extension; }  ///< return a reference to _extension object

      std::map<std::string, FitsKeys> &elmomap()   { return _elmo;      }

      void erase_extensions() { for ( auto &chan : _elmo ) chan.second.erase_db(); }

      void erase_extensions(const std::string &chan) {
        if ( _elmo.count(chan) > 0 ) _elmo.at(chan).erase_db();
        return;
      }

      FitsKeys &elmo(const std::string &chan) {
        try { return _elmo.at(chan); }
        catch ( const std::exception &e ) {
          logwrite("Common::Header::elmo", "ERROR "+chan+" not in elmo");
          static FitsKeys error;
          return error;
        }
      }
      const FitsKeys &elmo(const std::string &chan) const {
        try { return _elmo.at(chan); }
        catch ( const std::exception &e ) {
          logwrite("Common::Header::elmo", "ERROR "+chan+" not in elmo");
          static FitsKeys error;
          return error;
        }
      }

      void copy_extensionmap( const Header &source ) { this->_elmo = source._elmo; }

      void copy_extension( const std::string &chan, const Header &src ) {
        if ( src.has_chan(chan) ) this->_elmo[chan] = src.elmo(chan);
      }

      void merge_extension( const std::string &chan, const Header &src ) {
        // the source must have the requested channel
        //
        if ( src.has_chan(chan) ) {
          // if the destination has the same channel then use FitsKeys::merge
          // to merge the keydbs
          //
          if ( this->has_chan(chan) ) {
            this->_elmo[chan].merge( src.elmo(chan).keydb );
          }
          else {
            // otherwise copy the extension if it doesn't exist in the destination
            //
            this->_elmo[chan] = src.elmo(chan);
          }
        }
      }

      void add_extension(const std::string chan, const FitsKeys &keydb) { _elmo[chan] = keydb; }

      bool has_chan(const std::string chan) const { return _elmo.find(chan) != _elmo.end(); }

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

      template <typename T>
      void add_key( const std::string &keyword,
                    const T &value,
                    const std::string &comment,
                    bool use_extension,
                    const std::string &chan="") {
        const std::string function="Common::Header::add_key";
        std::stringstream message;

        if ( use_extension && !has_chan(chan) ) {
          FitsKeys newdb;
          add_extension(chan, newdb);
          message.str(""); message << "added extension chan " << chan << " to keydb for " << keyword << "=" << value << " / " << comment;
          logwrite( function, message.str() );
        }

        // Select the correct FitsKeys database. The default is primary()
        // but if chan is passed then use the appropriate FitsKey
        // object from the map.
        //
        FitsKeys &keydb = ( use_extension ? elmo(chan) : primary() );

///     message.str(""); message << "[DEBUG] will try to add " << keyword << "=" << value << " / " << comment
///                              << " to " << ( use_extension ? chan : "primary" );
///     logwrite(function,message.str());
        this->add_key( keydb, keyword, value, comment );
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
      void add_json_key( const json &jmessage,
                         const std::string &jkey,
                         const std::string &keyword,
                         const std::string &comment,
                         bool use_extension,
                         const std::string &chan="") {
        const std::string function="Common::Header::add_json_key";
        std::stringstream message;

        if ( use_extension && !has_chan(chan) ) {
          FitsKeys newdb;
          add_extension(chan, newdb);
          message.str(""); message << "added extension chan " << chan << " to keydb";
          logwrite( function, message.str() );
        }

        try {
          // extract the value from the JSON message using jkey as the key
          //
          auto jvalue = jmessage.at( jkey );

          // don't add null values
          //
          if ( jvalue == nullptr ) return;

          // Select the correct FitsKeys database. The default is primary()
          // but if chan is passed then use the appropriate FitsKey
          // object from the map.
          //
          FitsKeys &keydb = ( use_extension ? elmo(chan) : primary() );

          // type-check each value to call add_key with the correct type
          // keyword is the header keyword if not empty, otherwise use jkey
          //
          if ( jvalue.type() == json::value_t::boolean ) {
            this->add_key( keydb, (!keyword.empty()?keyword:jkey), jvalue.template get<bool>(), comment );
          }
          else
          if ( jvalue.type() == json::value_t::number_integer ) {
            this->add_key( keydb, (!keyword.empty()?keyword:jkey), jvalue.template get<int>(), comment );
          }
          else
          if ( jvalue.type() == json::value_t::number_unsigned ) {
            this->add_key( keydb, (!keyword.empty()?keyword:jkey), jvalue.template get<uint16_t>(), comment );
          }
          else
          if ( jvalue.type() == json::value_t::number_float ) {
            this->add_key( keydb, (!keyword.empty()?keyword:jkey), jvalue.template get<double>(), comment );
          }
          else
          if ( jvalue.type() == json::value_t::string ) {
            this->add_key( keydb, (!keyword.empty()?keyword:jkey), jvalue.template get<std::string>(), comment );
          }
          else {
            message << "ERROR unknown type for keyword " << (!keyword.empty()?keyword:jkey) << "=" << jvalue;
            logwrite( function, message.str() );
          }
        }
        catch( const json::exception &e ) {
          message.str(""); message << "JSON exception adding keyword " << (!keyword.empty()?keyword:jkey) << ": " << e.what();
          logwrite( function, message.str() );
        }
        catch( const std::exception &e ) {
          message.str(""); message << "ERROR exception adding keyword " << (!keyword.empty()?keyword:jkey) << ": " << e.what();
          logwrite( function, message.str() );
        }
      }

      void add_elmo_key( FitsKeys &keydb,
                         const json &jmessage,
                         const std::string &jkey,
                         const std::string &keyword,
                         const std::string &comment ) {
        logwrite("add_elmo_key","ERROR why are you calling me?");
      }

      template <typename HeaderType, typename T=nlohmann::json>
      void add_json_key( HeaderType type,
                         const T &jmessage,
                         const std::string &keyword, const std::string &comment ) {
        add_json_key( type, jmessage, keyword, keyword, comment );
        logwrite("add_json_key","ERROR why are you calling me?");
      }
      template <typename HeaderType>
      void add_json_key( HeaderType type,
                         const nlohmann::json &jmessage,
                         const std::string &jkey,
                         const std::string &keyword,
                         const std::string &comment ) {
        logwrite("add_json_key","ERROR why are you calling me?");
      }
      /***
        const std::string function="Common::Header::add_json_key";
        std::stringstream message;

        message.str(""); message << "[DEBUG] jkey=" << jkey << " keyword=" << keyword;
        logwrite(function,message.str());

        try {
          // extract the value from the JSON message using jkey as the key
          //
          auto jvalue = jmessage.at( jkey );

          // select the correct FitsKeys database from the Common::Header object,
          // primary or extension
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
      ***/
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
      long timeout;               ///< Poll timeout in ms
      bool timedout;
      std::atomic<int> num_send;

    public:
      DaemonClient()
        : term_write('\n'), term_read('\n'), term_with_string(false),
          timeout(POLLTIMEOUT), timedout(false), num_send(2), name("unconfigured_daemon"), port(-1), nbport(-1) {
        this->socket.sethost( "localhost" );
      };

      DaemonClient( std::string name_in )
        : term_write('\n'), term_read('\n'), term_with_string(false),
          timeout(POLLTIMEOUT), timedout(false), num_send(2), name(name_in), port(-1), nbport(-1) {
        this->socket.sethost( "localhost" );
      };   ///< preferred constructor with name to identify daemon

      DaemonClient( std::string name_in, char write_in, char read_in )
        : term_write(write_in), term_read(read_in), term_with_string(false),
          timeout(POLLTIMEOUT), timedout(false), num_send(2), name(name_in), port(-1), nbport(-1) {
        this->socket.sethost( "localhost" );
      };   ///< preferred constructor with name to identify daemon

      DaemonClient( std::string name_in, const std::string &write_in, const std::string &read_in )
        : term_str_write(write_in), term_str_read(read_in), term_with_string(true),
          timeout(POLLTIMEOUT), timedout(false), num_send(2), name(name_in), port(-1), nbport(-1) {
        this->socket.sethost( "localhost" );
      };   ///< construct with terminating strings

      ~DaemonClient() { this->disconnect(); }

      std::string name;             ///< name of the daemon
      std::string host;             ///< host where the daemon is running
      int port;                     ///< blocking port that the daemon is listening on
      int nbport;                   ///< non-blocking port that the daemon is listening on

      Network::TcpSocket socket;    ///< socket object for communications with daemon

      long async( std::string args );                              ///< async (non-blocking) commands to daemon that don't need a reply
      long async( std::string args, std::string &retstring );      ///< async (non-blocking) commands to daemon that need a reply

      static void dothread_command( Common::DaemonClient &daemon, std::string args );
      long command_timeout( std::string args, const long to );     ///< commands to daemon with timeout override
      long command_timeout( std::string args, std::string &retstring, const long to );     ///< commands to daemon with timeout override
      long command( std::string args );                            ///< commands to daemon
      long command( std::string args, std::string &retstring );    ///< commands to daemon that need a reply
      long send( std::string command,
                 std::string &reply,
                 const std::string &term_str_write_override="",
                 const std::string &term_str_read_override="",
                 bool term_with_string_override=false );
      long send( std::string command,
                 std::string &reply,
                 int timeout_in,
                 const std::string &term_str_write_override="",
                 const std::string &term_str_read_override="",
                 bool term_with_string_override=false );
      long connect();                                              ///< initialize socket connection to daemon
      void disconnect();                                           ///< close socket connection to daemon
      inline void set_name( const std::string &name_in ) { this->name=name_in; } ///< name this daemon
      inline void set_port( const int &_port ) { this->port=_port; }             ///< set the port number

      long is_connected( std::string &reply );
      bool poll_open();                                            ///< is device open?
      bool is_open();                                              ///< is device open?
      bool is_open(bool silent);                                   ///< is device open?
  };
  /***** Common::DaemonClient *********************************************************/

}
/***** Common *****************************************************************/
