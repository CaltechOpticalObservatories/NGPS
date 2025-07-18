/**
 * @file     common.cpp
 * @brief    Common namespace functions
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#include "common.h"

namespace Common {

  /***** Common::collect_telemetry ********************************************/
  /**
   * @brief      send the TELEMREQUEST command to daemon to get telemetry
   * @param[in]  provider   pair contains <"provider", port>
   * @param[out] retstring  serialized string of json telemetry message
   *
   */
  void collect_telemetry(const std::pair<std::string,int> &provider, std::string &retstring) {
    // Instantiate a client to communicate with each daemon,
    // constructed with no name, newline termination on command writes,
    // and JEOF termination on reply reads.
    //
    Common::DaemonClient jclient("", "\n", JEOF );

    // Send the command TELEMREQUEST to each daemon and read back the reply into
    // retstring, which will be the serialized JSON telemetry message.
    //
    jclient.set_name(provider.first);
    jclient.set_port(provider.second);
    jclient.connect();
    jclient.command(TELEMREQUEST, retstring);
    jclient.disconnect();

    return;
  }
  /***** Common::collect_telemetry ********************************************/


  /***** Common::Queue::enqueue ***********************************************/
  /**
   * @brief      puts a message into the queue
   * @param[in]  message  string to write
   *
   */
  void Queue::enqueue(std::string message) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    message_queue.push(message);
    notifier.notify_one();
    return;
  }
  /***** Common::Queue::enqueue ***********************************************/


  /***** Common::Queue::enqueue_and_log ***************************************/
  /**
   * @brief      puts a message into the queue and writes it to the log
   * @param[in]  function  name of function for logging purposes
   * @param[in]  message   string to write
   *
   */
  void Queue::enqueue_and_log(std::string function, std::string message) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    message_queue.push(message);
    notifier.notify_one();
    logwrite( function, message );
    return;
  }
  /***** Common::Queue::enqueue_and_log ***************************************/


  /***** Common::Queue::enqueue_and_log ***************************************/
  /**
   * @brief      puts a message into the queue and writes it to the log
   * @param[in]  tag       tag for broadcast message
   * @param[in]  function  name of function for logging purposes
   * @param[in]  message   string to write
   *
   */
  void Queue::enqueue_and_log( std::string tag, std::string function, std::string message ) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    std::stringstream qmessage;
    qmessage << tag << ":" << message;
    message_queue.push(qmessage.str());
    notifier.notify_one();
    logwrite( function, message );
    return;
  }
  /***** Common::Queue::enqueue_and_log ***************************************/


  /***** Common::Queue::dequeue ***********************************************/
  /**
   * @brief      pops the first message off the queue
   * @return     message  string read from the queue
   *
   * Get the "front"-element.
   * If the queue is empty, wait untill an element is avaiable.
   *
   */
  std::string Queue::dequeue(void) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    while(message_queue.empty()) {
      notifier.wait(lock);   // release lock as long as the wait and reaquire it afterwards.
    }
    std::string message = message_queue.front();
    message_queue.pop();
    return message;
  }
  /***** Common::Queue::dequeue ***********************************************/


  /***** Common::FitsKeys::get_keytype ****************************************/
  /**
   * @brief      return the keyword type based on the keyvalue
   * @param[in]  value   string to get the type
   * @return     string  one of the following: "BOOL", "STRING", "DOUBLE", "INT"
   *
   * This function looks at the contents of the value string to determine if it
   * contains an INT, DOUBLE, BOOL or STRING, and returns a string identifying the type.
   * That type is used in FITS_file::add_user_key() for adding keywords to the header.
   *
   */
  std::string FitsKeys::get_keytype(std::string keyvalue) {

    // strip leading and trailing spaces
    keyvalue.erase(0, keyvalue.find_first_not_of(' '));
    keyvalue.erase(keyvalue.find_last_not_of(' ')+1);

    // If it's empty then what else can it be but string
    if ( keyvalue.empty() ) return std::string( "STRING" );

    // if the entire string is either (exactly) T or F then it's a boolean
    if ( keyvalue == "T" || keyvalue == "F" ) {
      return std::string( "BOOL" );
    }

    // check the significand
    std::size_t pos(0);
    if (keyvalue[pos] == '+' || keyvalue[pos] == '-') ++pos;    // skip the sign if exist

    // count the number of numbers and number of decimal points
    // stops early if any other character is found, which makes the type STRING
    int numnum, numpts;
    for (numnum=0, numpts=0;
         pos<keyvalue.size() && (std::isdigit(keyvalue[pos]) || keyvalue[pos] == '.');
         ++pos) {
        keyvalue[pos] == '.' ? ++numpts : ++numnum;
    }

    if ( pos<keyvalue.size()  ||     // encountered non-num, non-point
         numpts>1             ||     // more than one decimal point
         numnum<1 ) {                // no numbers
      return std::string("STRING");  // then it's a string
    }
    else
    // one decimal is float or double...
    if (numpts==1 && numnum>0) {
      size_t loc = keyvalue.find('.');
      // get the integer and fractional parts of the number
      std::string  intpart = (loc==std::string::npos) ? keyvalue : keyvalue.substr(0, loc);
      std::string fracpart = (loc==std::string::npos) ? "" : keyvalue.substr(loc+1);

      // remove leading and trailing zeros
      intpart.erase(0, intpart.find_first_not_of('0'));
      fracpart.erase(fracpart.find_last_not_of('0'));

      // number of significant figures
      size_t sigfigs = intpart.length() + fracpart.length();

      // more than 7 significant figures requires a double
      return (sigfigs > 7 ? std::string("DOUBLE") : std::string("FLOAT"));
    }
    else
    // no decimal must be int or long
    if (numpts==0 && numnum>0) {
      try {
        // safely convert to long
        long long val = std::stoll(keyvalue);
        // if in range of int then use int, otherwise long
        if (val >= INT_MIN && val <= INT_MAX) return std::string("INT");
        else return std::string("LONG");
      }
      // failed conversion returns string
      catch (...) { return std::string("STRING"); }
    }

    // if this somehow falls through then return string
    return std::string("STRING");
  }
  /***** Common::FitsKeys::get_keytype ****************************************/


  /***** Common::FitsKeys::listkeys *******************************************/
  /**
   * @brief      list FITS keywords in internal database
   * @return     NO_ERROR
   *
   */
  long FitsKeys::listkeys() {
    std::string function = "Common::FitsKeys::listkeys";
    std::stringstream message;
    for ( const auto &keydb : this->keydb ) {
      message.str("");
      message << keydb.second.keyword << " = " << keydb.second.keyvalue;
      if ( ! keydb.second.keycomment.empty() ) message << " // " << keydb.second.keycomment;
      message << " (" << keydb.second.keytype << ")";
      logwrite(function, message.str());
    }
    return(NO_ERROR);
  }
  /***** Common::FitsKeys::listkeys *******************************************/


  /***** Common::FitsKeys::delkey *********************************************/
  /**
   * @brief      convenience function to delete FITS keyword from internal database
   * @details    simply calls addkey with a value="." which removes the keyword
   * @param[in]  key  string containing the keyword to remove
   * @return     ERROR or NO_ERROR, as returned by addkey()
   *
   */
  long FitsKeys::delkey( const std::string &key ) {
    const std::string function="Common::FitsKeys::delkey";
    std::stringstream message;

    fits_key_t::iterator it = this->keydb.find(key);

    if (it==this->keydb.end()) {
      message.str(""); message << "keyword " << key << " not found";
      logwrite(function, message.str());
    }
    else {
      this->keydb.erase(it);
      message.str(""); message << "keyword " << key << " erased";
      logwrite(function, message.str());
    }
    return NO_ERROR;
  }
  /***** Common::FitsKeys::delkey *********************************************/


  /***** Common::FitsKeys::addkey *********************************************/
  /**
   * @brief      add FITS keyword to internal database
   * @details    parses the input string and calls overloaded function addkey()
   * @param[in]  arg  string formatted as "KEYWORD=VALUE//COMMENT"
   * @return     ERROR for improper input arg, otherwise NO_ERROR
   *
   * This function is overloaded (a template function is defined in common.h).
   *
   * This version expects format of input arg as KEYWORD=VALUE//COMMENT
   * where COMMENT is optional. KEYWORDs are automatically converted to uppercase.
   *
   * Internal database is Common::FitsKeys::keydb
   * 
   */
  long FitsKeys::addkey( const std::string &arg ) {
    std::string function = "Common::FitsKeys::addkey";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::string keyword, keystring, keyvalue, keytype, keycomment;
    std::string comment_separator = "//";

    // There must be one equal '=' sign in the incoming string, so that will make two tokens here
    //
    Tokenize(arg, tokens, "=");
    if (tokens.size() != 2) {
      logwrite( function, "missing or too many '=': expected KEYWORD=VALUE//COMMENT (optional comment)" );
      return(ERROR);
    }

    keyword   = tokens[0].substr(0,8);                                     // truncate keyword to 8 characters
    keyword   = keyword.erase(keyword.find_last_not_of(" ")+1);            // remove trailing spaces from keyword
    std::locale loc;
    for (std::string::size_type ii=0; ii<keyword.length(); ++ii) {         // Convert keyword to upper case:
      keyword[ii] = std::toupper(keyword[ii],loc);                         // prevents duplications in STL map
    }
    keystring = tokens[1];                                                 // tokenize the rest in a moment

    size_t pos = keystring.find(comment_separator);                        // location of the comment separator
    keyvalue = keystring.substr(0, pos);                                   // keyvalue is everything up to comment

    size_t first = keyvalue.find_first_not_of(' ');
    if (first==std::string::npos) keyvalue.clear();
    else {
      size_t last = keyvalue.find_last_not_of(' ');
      keyvalue = keyvalue.substr(first, last-first+1);                     // substr removes leading/trailing spaces
    }

    if (pos != std::string::npos) {
      keycomment = keystring.erase(0, pos + comment_separator.length());
      keycomment = keycomment.erase(0, keycomment.find_first_not_of(" ")); // remove leading spaces from keycomment
    }

    // Delete the keydb entry for associated keyword if the keyvalue is a sole period '.'
    //
    if (keyvalue == ".") {
      fits_key_t::iterator ii = this->keydb.find(keyword);
      if (ii==this->keydb.end()) {
        message.str(""); message << "keyword " << keyword << " not found";
        logwrite(function, message.str());
      }
      else {
        this->keydb.erase(ii);
        message.str(""); message << "keyword " << keyword << " erased";
        logwrite(function, message.str());
      }
      return(NO_ERROR);
    }

    // check for instances of the comment separator in keycomment
    //
    if (keycomment.find(comment_separator) != std::string::npos) {
      message.str(""); message << "ERROR: FITS comment delimiter: found too many instancces of " << comment_separator << " in keycomment";
      logwrite( function, message.str() );
      return(NO_ERROR);
    }

    // Apply keyword rules one at a time so that the error can identify the problem
    //

    // no embedded spaces allowed in keyword
    //
    if ( keyword.find( " " ) != std::string::npos ) {
      message.str(""); message << "ERROR embedded spaces not allowed in keyword \"" << keyword << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    // keyword must start with a letter {A:Z}
    //
    static const std::regex start( "[A-Z].*" );  // start with [A-Z] followed by 0 or more of anything
    if ( !regex_match( keyword, start ) ) {
      message.str(""); message << "ERROR first character of keyword \"" << keyword << "\" must start with A:Z";
      logwrite( function, message.str() );
      return ERROR;
    }

    // keyword must contain only A-Z, 0-9, underscore and dash
    //
    static const std::regex contents( "[A-Z][A-Z0-9_-]*" );  // start with A-Z followed by zero or more of A-Z, 0-9, _, -
    if ( !regex_match( keyword, contents ) ) {
      message.str(""); message << "ERROR keyword \"" << keyword << "\" must contain only A-Z, 0-9, underscore or hyphen";
      logwrite( function, message.str() );
      return ERROR;
    }

    // keyword cannot conntain 0-padded number sequences
    //
    static const std::regex zeropadding( "[A-Z_-]*[0][0-9]+[A-Z_-]*" );  // zero or more chars and 0 followed by one or more numbers
    if ( regex_match( keyword, zeropadding ) ) {
      message.str(""); message << "ERROR keyword \"" << keyword << "\" cannot contain 0-padded numbers";
      logwrite( function, message.str() );
      return ERROR;
    }

    // keyvalue limited to 68 bytes
    //
    if ( keyvalue.length() > 68 ) {
      message.str(""); message << "ERROR keyvalue \"" << keyvalue << "\" cannot exceed 68 bytes";
      logwrite( function, message.str() );
      return ERROR;
    }

    // truncate comments if too long and warn only, no error
    //
    if ( keyvalue.length() + keycomment.length() > 68 ) {
      int limit = 68 - keyvalue.length();
      keycomment = keycomment.substr(0, limit);
      message.str(""); message << "NOTICE comment truncated to fit record limit: \"" << keycomment << "\"";
      logwrite( function, message.str() );
    }

    // insert new entry into the database using the overloaded function
    //
    this->addkey( keyword, keyvalue, keycomment );

    return NO_ERROR;
  }
  /***** Common::FitsKeys::addkey *********************************************/


  /***** Common::FitsKeys::addkey *********************************************/
  /**
   * @brief      add FITS keyword to internal database
   * @details    parses the input string and calls overloaded function addkey()
   * @param[in]  vec  vector containing strings for KEYWORD, VALUE and optional COMMENT
   * @return     ERROR for improper input arg, otherwise NO_ERROR
   *
   * This function is overloaded
   *
   * This version expects a vector containing 2 or 3 elements, in the order of
   * { KEYWORD, VALUE, [COMMENT] }.
   *
   * Internal database is Common::FitsKeys::keydb
   * 
   */
  long FitsKeys::addkey( const std::vector<std::string> &vec ) {
    std::string function = "Common::FitsKeys::addkey";
    std::stringstream message;
    std::string key, val, com;

    auto vsize = vec.size();

    // input vector must have 2 or 3 elements
    //
    if ( vsize < 2 || vsize > 3 ) {
      message << "ERROR: bad number of elements in vector: " << vsize << ". expected 2 or 3";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // assign each element of the input vector to a string
    //
    try {
      key = vec.at(0);
      val = vec.at(1);
      com = ( vsize==3 ? vec.at(2) : "" );
    }
    catch ( std::out_of_range &e ) {
      logwrite( function, "ERROR: exception parsing vector" );
      return( ERROR );
    }

    // use the template class version of addkey() with these three strings
    //
    return this->addkey( key, val, com );
  }
  /***** Common::FitsKeys::addkey *********************************************/


///***** Common::DaemonClient::configure **********************************************/
///**
// * @brief      configure the (private) variables for daemon communication
// * @param[in]  port  port of daemon (int)
// *
// */
//void Common::DaemonClient::configure( int port ) {
//  std::string function = "DaemonClient::configure";
//  std::stringstream message;
//  this->port = port;
//  message << this->name << " configured with port " << this->port;
//  logwrite( function, message.str() );
//  return;
//}
///***** Common::DaemonClient::configure **********************************************/


  /***** Common::DaemonClient::async **************************************************/
  /**
   * @brief      async (non-blocking) commands to daemon that don't need a reply
   * @param[in]  command  string command
   * @return     ERROR or NO_ERROR
   *
   */
  long Common::DaemonClient::async( std::string command ) {
    std::string noreply="NOREPLY";           // this tells the next function to not wait for a reply
    return this->async( command, noreply );
  }
  /***** Common::DaemonClient::async **************************************************/


  /***** Common::DaemonClient::async **************************************************/
  /**
   * @brief      async (non-blocking) commands to daemon that need a reply
   * @param[in]  command  string command
   * @param[out] reply    reference to string to contain reply
   * @return     ERROR or NO_ERROR
   *
   * If the reply in string is pre-set to "NOREPLY" then send the command and return
   * immediately; do not wait for a reply.
   *
   */
  long Common::DaemonClient::async( std::string command, std::string &reply ) {
    std::string function = "Common::DaemonClient::async";
    std::stringstream message;
    long error = NO_ERROR;

    // Create a local socket object for non-blocking communication with the daemon
    //
    Network::TcpSocket _sock( "127.0.0.1", this->nbport );

    // Create and connect to the non-blocking socket
    //
    if ( _sock.Connect() < 0 ) {
      message.str(""); message << "ERROR connecting to " << this->name << " on localhost/" << this->nbport;
      logwrite( function, message.str() );
      return( ERROR );
    } else {
      message.str(""); message << "connected to " << this->name << " on localhost/" << this->nbport;
      logwrite( function, message.str() );
    }

    // Send the command to the non-blocking socket
    //
    message.str(""); message << "sending \"" << strip_newline(command) << "\" to " << this->name << " localhost/" << this->nbport;
    logwrite( function, message.str() );
    command.append( "\n" );
    int wrote = _sock.Write( command );

    if ( wrote <= 0 ) {
      message.str(""); message << "ERROR no bytes written for \"" << strip_newline(command) << "\" to " << this->name << " localhost/" << this->nbport;
      logwrite( function, message.str() );
      error = ERROR;
    }

    // If the reply in string has been set to "NOREPLY" then close the socket and return immediately.
    // Do not wait for a reply.
    //
    if ( reply == "NOREPLY" ) {
      message.str(""); message << "not waiting for reply and closing connection to " << this->name << " socket " << _sock.gethost()
                               << "/" << _sock.getport() << " on fd " << _sock.getfd();
      logwrite( function, message.str() );
      _sock.Close();
      return( error );
    }

    // Wait (poll) connected socket for incoming data...
    //
    int pollret;
    if ( error==NO_ERROR && ( ( pollret = _sock.Poll() ) <= 0 ) ) {
      if ( pollret == 0 ) {
        message.str(""); message << "TIMEOUT polling socket " << _sock.gethost() << "/" << _sock.getport()
                                 << " on fd " << _sock.getfd() << " for " << this->name;
        logwrite( function, message.str() );
      }
      if ( pollret <0 ) {
        message.str(""); message << "ERROR polling socket " << _sock.gethost() << "/" << _sock.getport()
                                 << " on fd " << _sock.getfd() << " for " << this->name << ": " << strerror(errno);
        logwrite( function, message.str() );
      }
      error = ERROR;
    }

    // read the response
    //
    char delim = '\n';
    long ret;
    if ( error==NO_ERROR && ( ( ret = _sock.Read( reply, delim ) ) <= 0 ) ) {
      if ( ret < 0 && errno != EAGAIN ) {             // could be an actual read error
        message.str(""); message << "ERROR reading from socket " << _sock.gethost() << "/" << _sock.getport()
                                 << " on fd " << _sock.getfd() << " for " << this->name << ": " << strerror(errno);
        logwrite(function, message.str());
      }
      if ( ret==0 ) {
        message.str(""); message << "TIMEOUT reading from socket " << _sock.gethost() << "/" << _sock.getport()
                                 << " on fd " << _sock.getfd() << " for " << this->name;
        logwrite( function, message.str() );
      }
      error = ERROR;
    }

    // close the connection
    //
    message.str(""); message << "closing connection to " << this->name << " socket " << _sock.gethost()
                             << "/" << _sock.getport() << " on fd " << _sock.getfd();
    logwrite( function, message.str() );
    _sock.Close();

    // assign the response to the reply string, passed in by reference
    //
    reply.erase( std::remove(reply.begin(), reply.end(), '\r' ), reply.end() );
    reply.erase( std::remove(reply.begin(), reply.end(), '\n' ), reply.end() );

    // If the reply contains "ERROR" then return ERROR, otherwise NO_ERROR.
    //
    if ( reply.find( std::string( "ERROR" ) ) != std::string::npos ) error = ERROR;

    return( error );
  }
  /***** Common::DaemonClient::async **************************************************/


  /***** Common::DaemonClient::send ***************************************************/
  /**
   * @brief      send a command, read the reply
   * @param[in]  command                    string command
   * @param[out] reply                      reference to string to contain reply
   * @param[in]  term_str_write_override    optional allows overriding class value
   * @param[in]  term_str_read_override     optional allows overriding class value
   * @param[in]  term_with_string_override  optional allows overriding class value
   * @return     ERROR | NO_ERROR
   *
   * this->term_write is automatically appended to the end of the command
   *
   * If Poll() returns < 0 but errno is not set then it's possible the file descriptor
   * is stale. If that happens, re-open the socket and try send()ing again. Limit the
   * number of retries which is set when the socket is initially opened.
   *
   * term_str_write and _read are set in the class ahead of time, but you can
   * override them for this call only by passing in different (optional) values.
   *
   */
  long Common::DaemonClient::send( std::string command,
                                   std::string &reply,
                                   const std::string &term_str_write_override,
                                   const std::string &term_str_read_override,
                                   bool term_with_string_override ) {
    return send( command, reply, this->timeout, term_str_write_override, term_str_read_override, term_with_string_override );
  }
  long Common::DaemonClient::send( std::string command,
                                   std::string &reply,
                                   int timeout_in,
                                   const std::string &term_str_write_override,
                                   const std::string &term_str_read_override,
                                   bool term_with_string_override ) {
    std::string function = "Common::DaemonClient::send";
    std::stringstream message;
    long ret;

    std::unique_lock<std::mutex> lock( this->client_access );

    if ( ! this->socket.isconnected() ) {
      message.str(""); message << "ERROR:cannot send \"" << strip_newline(command) << "\" to " << this->name
                               << " because daemon is not connected";
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( this->socket.getfd() < 1 ) {
      message.str(""); message << "ERROR:cannot send \"" << strip_newline(command) << "\" to " << this->name 
                               << " because fd " << this->socket.getfd() << " is invalid";
      logwrite( function, message.str() );
      return ERROR;
    }

    // Determine whether to use the override values or not
    // for this call only.
    //
    bool term_with_string_actual = term_with_string_override || this->term_with_string;

    const std::string &term_str_write_actual = term_str_write_override.empty() ? this->term_str_write
                                                                               : term_str_write_override;

    const std::string &term_str_read_actual = term_str_read_override.empty() ? this->term_str_read
                                                                             : term_str_read_override;

    // send the command
    //
    if ( term_with_string_actual ) {
      command += term_str_write_actual;
    }
    else {
      command += this->term_write;
    }

    int trys=0;
    int retry_limit=3;
    int pollret=0;

    while ( trys++ < retry_limit ) {

      this->timedout=false;

      // send the command
      //
      this->socket.Write( command );

      // This indicates that the caller only wants to send the command,
      // and doesn't want to read the reply.
      //
      if (reply=="DONTWAIT") return NO_ERROR;

      // Wait (poll) connected socket for incoming data...
      //
      pollret = this->socket.Poll(timeout_in);

      // got data so break out of retry loop
      //
      if ( pollret > 0 ) break;

      long error = ERROR;
      if ( pollret == 0 ) {
        message.str(""); message << "TIMEOUT polling socket " << this->socket.gethost() << "/" << this->socket.getport()
                                 << " on fd " << this->socket.getfd() << " for " << this->name;
        logwrite( function, message.str() );
        this->timedout=true;
      }
      else
      if ( pollret < 0 && errno ) {                // this is probably a real error
        message.str(""); message << "ERROR polling socket " << this->socket.gethost() << "/" << this->socket.getport()
                                 << " on fd " << this->socket.getfd() << " for " << this->name << ": " << strerror(errno);
        logwrite( function, message.str() );
      }
      else
      if ( pollret < 0 && !errno ) {               // this is probably a stale fd
        message.str(""); message << "ERROR no response from " << this->socket.gethost() << "/" << this->socket.getport()
                                 << ": will re-open connection to " << this->name << " and try again";
        logwrite( function, message.str() );
      }

      // if still here then reconnect, sleep 1s, try again
      //
      lock.unlock();
      error = this->connect();
      lock.lock();
      std::this_thread::sleep_for( std::chrono::seconds(1) );

    } // end retry while loop

    // out of retry loop, did poll succeed?
    //
    if ( pollret > 0 ) {
      // read the response, using either term_read or term_str_read as the terminator
      //
      ret = ( term_with_string_actual ? socket.Read( reply, term_str_read_actual ) : socket.Read( reply, term_read ) );

      if ( ret <= 0 ) {
        if ( ret < 0 && errno != EAGAIN ) {             // could be an actual read error
          message.str(""); message << "ERROR reading from socket " << this->socket.gethost() << "/" << this->socket.getport()
                                   << " on fd " << this->socket.getfd() << " for " << this->name << ": " << strerror(errno);
          logwrite(function, message.str());
        }
        if ( ret==0 ) {
          message.str(""); message << "TIMEOUT reading from socket " << this->socket.gethost() << "/"<< this->socket.getport()
                                   << " on fd " << this->socket.getfd() << " for " << this->name;
          logwrite( function, message.str() );
          this->timedout=true;
        }
      }
    }

    // If a timeout on a socket occured then it's possible that a response came after the timeout expired,
    // in which case, that old response is still waiting to be read, which is a response to an old command
    // that is no longer pertinent.
    //
    if ( this->timedout ) {
      logwrite( function, "[TEST] attempting to flush after timeout" );
      if ( ( pollret = this->socket.Poll(2000) ) > 0 ) {
        reply.erase( std::remove(reply.begin(), reply.end(), '\r' ), reply.end() );
        reply.erase( std::remove(reply.begin(), reply.end(), '\n' ), reply.end() );
        message.str(""); message << "[TEST] I read this: " << reply << " but I'm going to read again!";
        logwrite( function, message.str() );
        ret = ( term_with_string_actual ? socket.Read( reply, term_str_read_actual ) : socket.Read( reply, term_read ) );
        reply.erase( std::remove(reply.begin(), reply.end(), '\r' ), reply.end() );
        reply.erase( std::remove(reply.begin(), reply.end(), '\n' ), reply.end() );
        message.str(""); message << "[TEST] and the 2nd read was this: " << reply;
        logwrite( function, message.str() );
      }
      this->timedout=false;
    }

    // remove the read terminator
    //
    if ( term_with_string_actual ) {
      size_t termpos = reply.find(term_str_read_actual);
      if ( termpos != std::string::npos ) reply.erase(termpos);
    }
    else {
      size_t termpos = reply.find(term_read);
      if ( termpos != std::string::npos ) reply.erase(termpos);
    }

    // remove all carriage returns and newlines -- @TODO really?
    //
    reply.erase( std::remove(reply.begin(), reply.end(), '\r' ), reply.end() );
    reply.erase( std::remove(reply.begin(), reply.end(), '\n' ), reply.end() );

    // If the reply contains "ERROR" then return ERROR, otherwise NO_ERROR.
    //
    if ( reply.find( std::string( "ERROR" ) ) != std::string::npos ) {
      return( ERROR );
    }
    else return( NO_ERROR );
  }
  /***** Common::DaemonClient::send ***************************************************/


  /***** Common::DaemonClient::dothread_command ***************************************/
  /**
   * @brief      sends a command to a daemon in a thread
   * @param[in]  daemon  reference to Common::DaemonClient object
   * @param[in]  args    string containing command and any arguments
   *
   * This thread can be spawned so that the command can be handled in the background
   * without the caller having to wait for it to complete. Since there is no waiting
   * for the command to complete, the caller must use other means, such as monitoring
   * the UDP broadcast message port, for any potential status.
   *
   */
  void Common::DaemonClient::dothread_command( DaemonClient &daemon, std::string args ) {
    daemon.command( args );
    return;
  }
  /***** Common::DaemonClient::dothread_command ***************************************/


  /***** Common::DaemonClient::command_timeout ****************************************/
  /**
   * @brief      special daemon commands or send any command, no reply, timeout override
   * @details    temporarily override the default Poll timeout with the to provided,
   *             for this command only, then restores class timeout to default
   * @param[in]  args  string to send
   * @param[in]  to    timeout in ms
   * @return     ERROR | NO_ERROR
   *
   */
  long Common::DaemonClient::command_timeout( std::string args, const long to ) {
    long timeout_og = this->timeout;
    this->timeout = to;
    long error = this->command( args );
    this->timeout = timeout_og;
    return error;
  }
  /***** Common::DaemonClient::command_timeout ****************************************/


  /***** Common::DaemonClient::command_timeout ****************************************/
  /**
   * @brief      special daemon commands or send any command, with reply, timeout override
   * @details    temporarily override the default Poll timeout with the to provided,
   *             for this command only, then restores class timeout to default
   * @param[in]  args       string to send
   * @param[out] retstring  reference to string for reply
   * @param[in]  to         timeout in ms
   * @return     ERROR | NO_ERROR
   *
   */
  long Common::DaemonClient::command_timeout( std::string args, std::string &retstring, const long to ) {
    long timeout_og = this->timeout;
    this->timeout = to;
    long error = this->command( args, retstring );
    this->timeout = timeout_og;
    return error;
  }
  /***** Common::DaemonClient::command_timeout ****************************************/


  /***** Common::DaemonClient::command ************************************************/
  /**
   * @brief      special daemon commands or send any command, no reply
   * @param[in]  args  string to send
   * @return     ERROR or NO_ERROR
   *
   * special commands { connect disconnect isconnected }
   *
   */
  long Common::DaemonClient::command( std::string args ) {
    std::string function = "Common::DaemonClient::command";
    std::stringstream message;
    std::string reply;
    std::string done = "DONE";
    long retval = this->command( args, reply );
    if ( reply.find( std::string( "DONE" ) ) == std::string::npos ) {
      message.str(""); message << "sending " << strip_newline(args) << " to " << this->name << " returned " << reply;
      logwrite( function, message.str() );
    }
#ifdef LOGLEVEL_DEBUG
    else {
      message.str(""); message << "[DEBUG] sending " << strip_newline(args) << " to " << this->name << " returned " << reply;
      logwrite( function, message.str() );
    }
#endif
    return( retval );
  }
  /***** Common::DaemonClient::command ************************************************/


  /***** Common::DaemonClient::command ************************************************/
  /**
   * @brief      special daemon commands or send any command, return reply
   * @param[in]  args       string to send
   * @param[out] retstring  reference to string to hold reply
   * @return     ERROR or NO_ERROR
   *
   * special commands { connect disconnect isconnected }
   *
   */
  long Common::DaemonClient::command( std::string args, std::string &retstring ) {
    std::string function = "Common::DaemonClient::command";
    std::stringstream message;
    long error = NO_ERROR;

    // this is probably a programming error if we're in here and port is undefined
    //
    if ( this->port < 0 ) {
      message.str(""); message << "daemon \"" << this->name << "\" not configured";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // connect to the daemon
    //
    if ( args == "connect" ) {
      // initialize socket connection
      //
      if ( ( error = this->connect() ) != NO_ERROR ) retstring="ERROR"; else retstring="DONE"; 
#ifdef LOGLEVEL_DEBUG
//    message.str(""); message << "[DEBUG] connected to " << this->name << " socket " << this->socket.gethost()
//                             << "/" << this->socket.getport() << " on fd " << this->socket.getfd();
//    logwrite( function, message.str() );
#endif
    }
    else

    if ( args == "isconnected" ) {
      error = this->is_connected( retstring );
    }
    else

    // disconnect from the daemon
    //
    if ( args == "disconnect" ) {
#ifdef LOGLEVEL_DEBUG
//    message.str(""); message << "[DEBUG] disconnecting " << this->name << " socket " << this->socket.gethost()
//                             << "/" << this->socket.getport() << " from fd " << this->socket.getfd();
//    logwrite( function, message.str() );
#endif
      // then close the connection
      //
      this->socket.Close();
      retstring="DONE";
    }

    // all other commands go straight on through, as-is
    //
    else {
      // but only if the connection is open of course
      //
      if ( !this->socket.isconnected() ) {
        message.str(""); message << "ERROR: connection not open to " << this->name;
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {
#ifdef LOGLEVEL_DEBUG
//      message.str(""); message << "[DEBUG] sending to " << this->name << " socket " << this->socket.gethost()
//                               << "/" << this->socket.getport() << " on fd " << this->socket.getfd() << ": " << args;
//      logwrite( function, message.str() );
#endif
        error = this->send( args, retstring );
      }
    }

#ifdef LOGLEVEL_DEBUG
//  message.str(""); message << "[DEBUG] reply from " << this->name << " socket " << this->socket.gethost()
//                           << "/" << this->socket.getport() << " on fd " << this->socket.getfd() << ": " << retstring;
//  logwrite( function, message.str() );
#endif

    return( error );
  }
  /***** Common::DaemonClient::command ************************************************/


  /***** Common::DaemonClient::connect ************************************************/
  /**
   * @brief      initialize socket connection to the daemon
   * @details    establishes socket connection to daemon using Network::TcpSocket class
   * @return     ERROR or NO_ERROR
   *
   */
  long Common::DaemonClient::connect() {
    const std::string function = "Common::DaemonClient::connect";
    std::stringstream message;
    long error = NO_ERROR;

    const std::lock_guard<std::mutex> lock( this->client_access );

    // probably a programming error if this Common::DaemonClient object is not configured
    //
    if ( this->port == -1 ) {
      logwrite( function, "ERROR port not configured for "+this->name );
      error = ERROR;
    }
    else {

      // If already connected then close the connection
      //
      if ( this->socket.isconnected() ) {
        message.str(""); message << "closing existing connection to " << this->name << " socket " << this->socket.gethost()
                                 << "/" << this->socket.getport() << " on fd " << this->socket.getfd();
        logwrite( function, message.str() );
        this->socket.Close();
      }

      this->socket.setport( this->port );

      // Create and connect to the socket
      //
      if ( this->socket.Connect() < 0 ) {
        logwrite( function, "ERROR connecting to "+this->name+" on port "+std::to_string(this->port) );
        error = ERROR;
      } else {
        message.str(""); message << "connected to " << this->name << " at " << this->socket.gethost() << "/" << this->socket.getport()
                                 << " on fd " << this->socket.getfd();
        logwrite( function, message.str() );
      }
    }

    return error;
  }
  /***** Common::DaemonClient::connect ************************************************/


  /***** Common::DaemonClient::disconnect *********************************************/
  /**
   * @brief      close socket connection to the daemon
   *
   * This function closes the socket connection to the daemon
   * using the Network::TcpSocket class.
   *
   */
  void Common::DaemonClient::disconnect() {
    std::string function = "Common::DaemonClient::disconnect";
    std::stringstream message;

    const std::lock_guard<std::mutex> lock( this->client_access );

    // close the connection
    //
    message.str(""); message << "closing connection to " << this->name << " socket " << this->socket.gethost()
                             << "/" << this->socket.getport() << " on fd " << this->socket.getfd();
    logwrite( function, message.str() );
    this->socket.Close();

    return;
  }
  /***** Common::DaemonClient::disconnect *********************************************/


  /***** Common::DaemonClient::is_connected *******************************************/
  /**
   * @brief      return the connected state of a socket connection to the daemon
   * @param[out] reply  reference to string = "true" | "false"
   * @return     ERROR or NO_ERROR
   *
   * This function establishes a socket connection to the daemon
   * using the Network::TcpSocket class.
   *
   */
  long Common::DaemonClient::is_connected( std::string &reply ) {
    std::string function = "Common::DaemonClient::is_connected";
    std::stringstream message;

    reply = ( this->socket.isconnected() ? "true" : "false" );

    message << this->name << " is" << ( this->socket.isconnected() ? " " : " not " ) << "connected";
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /***** Common::DaemonClient::is_connected *******************************************/


  /***** Common::DaemonClient::is_open ****************************************/
  /**
   * @brief      check if daemon has opened its device
   * @details    This sends an "isopen" command to the daemon, which is used
   *             to have the daemon report if it has opened a connection to
   *             whatever device it controls. This is not the state of the socket
   *             connection to the daemon, but the daemon's connection to its hardware.
   * @return     true | false
   *
   */
  bool Common::DaemonClient::poll_open() {
    return is_open(true);
  }
  bool Common::DaemonClient::is_open() {
    return is_open(false);
  }
  bool Common::DaemonClient::is_open( bool silent ) {
    std::string function = "Common::DaemonClient::is_open";
    std::stringstream message;
    std::string reply;
    bool state=false;

    if ( !this->socket.isconnected() ) {
      message.str(""); message << "ERROR not connected to " << this->name
                               << " on " << this->host << ":" << this->port;
      return( false );
    }

    if ( this->send( (silent?"poll isopen":"isopen"), reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending \"isopen\" to " << this->name
                               << " on " << this->host << ":" << this->port;
      logwrite( function, message.str() );
      return( false );
    }

    // Tokenize the reply --
    // There must be at least one token and the first (or only) token
    // is expected to be {true|false}. The second (or last) token is
    // expected to be {DONE|ERROR}.
    //
    std::vector<std::string> tokens;
    Tokenize( reply, tokens, " " );
    if ( tokens.empty() ) {
      message.str(""); message << "ERROR no response from " << this->name
                               << " on " << this->host << ":" << this->port;
      logwrite( function, message.str() );
      return( false );
    }

    // If we're still here then just 2 tokens, good.
    // Parse the tokens to get the homed state and set a flag if homing is needed..
    //
    if ( tokens.back() != "DONE" ) {
      message.str(""); message << "ERROR reading open state from " << this->name;
      logwrite( function, message.str() );
      return( false );
    }
    if ( tokens.front() == "true" ) state  = true;
    else
    if ( tokens.front() == "false" ) state = false;
    else {
      message.str(""); message << "ERROR unknown state \"" << tokens.front()
                               << "\" from " << this->name << ": expected {true|false}";
      logwrite( function, message.str() );
      return( false );
    }
  return( state );
  }
  /***** Common::DaemonClient::is_open ****************************************/


}
