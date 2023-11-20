/**
 * @file     common.cpp
 * @brief    Common namespace functions
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#include "common.h"

namespace Common {

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
    std::size_t pos(0);

    // if the entire string is either (exactly) T or F then it's a boolean
    if ( keyvalue == "T" || keyvalue == "F" ) {
      return std::string( "BOOL" );
    }

    // skip the whitespaces
    pos = keyvalue.find_first_not_of(' ');
    if (pos == keyvalue.size()) return std::string("STRING");   // all spaces, so it's a string

    // check the significand
    if (keyvalue[pos] == '+' || keyvalue[pos] == '-') ++pos;    // skip the sign if exist

    // count the number of digits and number of decimal points
    int n_nm, n_pt;
    for (n_nm = 0, n_pt = 0; std::isdigit(keyvalue[pos]) || keyvalue[pos] == '.'; ++pos) {
        keyvalue[pos] == '.' ? ++n_pt : ++n_nm;
    }

    if (n_pt>1 || n_nm<1 || pos<keyvalue.size()){ // no more than one point, no numbers, or a non-digit character
      return std::string("STRING");               // then it's a string
    }

    // skip the trailing whitespaces
    while (keyvalue[pos] == ' ') {
        ++pos;
    }

    if (pos == keyvalue.size()) {
      if (keyvalue.find(".") == std::string::npos)    // all numbers and no decimals, it's an integer
        return std::string("INT");
      else                                            // otherwise numbers with a decimal, it's a double
        return std::string("DOUBLE");
    }
    else return std::string("STRING");                // lastly, must be a string
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
    fits_key_t::iterator keyit;
    for (keyit  = this->keydb.begin();
         keyit != this->keydb.end();
         keyit++) {
      message.str("");
      message << keyit->second.keyword << " = " << keyit->second.keyvalue;
      if ( ! keyit->second.keycomment.empty() ) message << " // " << keyit->second.keycomment;
      message << " (" << keyit->second.keytype << ")";
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
    return this->addkey( std::vector<std::string> { key, ".", "" } );
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
    keyvalue = keyvalue.erase(0, keyvalue.find_first_not_of(" "));         // remove leading spaces from keyvalue
    keyvalue = keyvalue.erase(keyvalue.find_last_not_of(" ")+1);           // remove trailing spaces from keyvalue
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

    // insert new entry into the database using the overloaded function
    //
    this->addkey( keyword, keyvalue, keycomment );

    return(NO_ERROR);
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

}
