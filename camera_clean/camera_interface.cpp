#include "camera_interface.h"
#include "camera_server.h"     // needed for full definition of Server and its members

namespace Camera {

  // this is the code for shared functions common to all implementations

  void Interface::set_server(Camera::Server* s) {
    this->server=s;
  }

  /***** Camera::Interface::handle_queue **************************************/
  /**
   * @brief      inserts a message into the asynchronous message queue
   * @param[in]  message  string to insert into the async message queue
   *
   * This should be spawned in a thread. This is how threads get messages
   * into the async message queue.
   *
   */
  void Interface::handle_queue(std::string message) {
    async.enqueue( message );
    return;
  }
  /***** Camera::Interface::handle_queue **************************************/


  void Interface::func_shared() {
    std::string function("Camera::Interface::func_shared");
    logwrite(function, "common implementation function");
  }


  /***** Camera::Interface::log_error *****************************************/
  /**
   * @brief      logs the error and saves the message to be returned on the command port
   * @param[in]  function  string containing the name of the Namespace::Class::function
   * @param[in]  message   string containing error message
   * @return     ERROR or NO_ERROR
   *
   */
  void Interface::log_error(const std::string &function, const std::string &message ) {
    std::stringstream err;

    // Save this message in class variable
    lasterrorstring.str("");
    lasterrorstring << message;

    // Form an error string as "ERROR: <message>"
    err << "ERROR: " << lasterrorstring.str();

    // Log and send to async port in the usual ways
    logwrite(function, err.str());
    async.enqueue(err.str());
  }
  /***** Camera::Interface::log_error *****************************************/


  /***** Camera::Interface::configure_constkeys *******************************/
  /**
   * @brief      processes the CONSTKEY_* keywords from the config file
   * @details    constant keywords allow insertion into all FITS files
   *             header keywords that won't change
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::configure_constkeys() {
    const std::string function("Camera::Interface::configure_constkeys");
    std::stringstream message;
    int applied=0;
    long error=NO_ERROR;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < server->config.n_entries; entry++) {

      // CONSTKEY_*
      //
      if (server->config.param[entry].compare(0, 9, "CONSTKEY_")==0) {
        // convert the arg into a vector and use the vector form of addkey()
        //
        std::vector<std::string> tokens;
        Tokenize( server->config.arg[entry], tokens, " " );

        if ( server->config.param[entry].compare( 9, 3, "PRI" )==0 ) error = this->camera_info.systemkeys.primary().addkey( tokens );
        else
        if ( server->config.param[entry].compare( 9, 3, "EXT" )==0 ) this->camera_info.systemkeys.add_key(tokens[0],tokens[1],"",true,"all");
        else
        continue;

        if ( error == ERROR ) {
          message.str(""); message << "ERROR: parsing config " << server->config.param[entry] << "=" << server->config.arg[entry];
          logwrite( function, message.str() );
          return ERROR;
        }

        message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

    } // end loop through the entries in the configuration file

    if (applied>0) {
      message.str(""); message << "applied " << applied << " constant FITS keys";
      error==NO_ERROR ? logwrite(function, message.str()) : log_error(function, message.str());
    }

    return error;
  }
  /***** Camera::Interface::configure_constkeys *******************************/


  long Interface::configure_serverkey(const std::string &key, const std::string &value, const std::string &comment) {
    return camera_info.systemkeys.primary().addkey(key, value, comment);
  }


  /***** Camera::Interface::disconnect_controller *****************************/
  /**
   * @brief      disconnect camera controller
   * @details    use this to disconnect before exiting because it takes
   *             no arguments and returns nothing
   *
   */
  void Interface::disconnect_controller() {
    std::string retstring;
    this->disconnect_controller("", retstring);
  }
  /***** Camera::Interface::disconnect_controller *****************************/


  void Interface::set_dirmode( mode_t mode ) {
    this->dirmode = mode;
  }

  long Interface::imdir( std::string args, std::string &retstring ) {
    const std::string function("Camera::Interface::imdir");
    return ERROR;
  }


  /***** Camera::Interface::longerror *****************************************/
  /**
   * @brief      set or get the longerror state
   * @param[in]  args       string containing requested state "true" or "false"
   * @param[out] retstring  reference to string containing the current state "true" or "false"
   * @return     true or false
   *
   * This function is overloaded.
   *
   */
  long Interface::longerror(std::string args, std::string &retstring) {
    const std::string function("Camera::Interface::longerror");
    std::stringstream message;
    int error = NO_ERROR;

    // If something is passed then try to use it to set the longerror state
    //
    if ( !args.empty() ) {
      try {
        std::transform( args.begin(), args.end(), args.begin(), ::tolower );    // make lowercase
        if (args == "false" ) this->is_longerror = false;
        else
        if (args == "true"  ) this->is_longerror = true;
        else {
          message.str(""); message << args << " is invalid. Expecting true or false";
          log_error( function, message.str() );
          error = ERROR;
        }
      }
      catch (...) {
        message.str(""); message << "unknown exception parsing argument: " << args;
        log_error( function, message.str() );
        error = ERROR;
      }
    }

    // error or not, the state reported is whatever was last successfully set
    //
    retstring = (this->is_longerror ? "true" : "false");
    logwrite( function, retstring );
    message.str(""); message << "NOTICE:longerror=" << retstring;
    this->async.enqueue( message.str() );

    // and this lets the server know if it was set or not
    //
    return error;
  }
  /***** Camera::Interface::longerror *****************************************/


  /***** Camera::Interface::set_shutter_delay *********************************/
  /**
   * @brief      interface for setting shutter delay from input string
   * @details    This uses the overloaded version which accepts a long and
   *             does the range checks.
   * @param[in]  arg  string representation of shutter delay in ms
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_shutter_delay( const std::string arg ) {
    try {
      return set_shutter_delay( std::stod( arg ) );
    }
    catch( const std::exception &e ) {
      logwrite("Camera::Interface::set_shutter_delay", "ERROR parsing "+arg+": "+std::string(e.what()));
      return ERROR;
    }
  }
  /***** Camera::Interface::set_shutter_delay *********************************/


  /***** Camera::Interface::set_shutter_delay *********************************/
  /**
   * @brief      interface for setting the shutter delay
   * @param[in]  arg  requested shutter delay in milliseconds
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_shutter_delay( long arg ) {
    std::string function = "Camera::Interface::set_shutter_delay";
    std::stringstream message;

    // If shutter delay within range {0:MAX} then save it to the class
    // and broadcast the change.
    //
    if ( arg < 0 || arg > MAX_SHUTTER_DELAY ) {
      message << "ERROR " << arg << " outside range { 0 : " << MAX_SHUTTER_DELAY << " } ms";
      logwrite( function, message.str() );
      return ERROR;
    }

    this->shutter_delay = arg;

    message << "changed SHUTTER_DELAY = " << this->shutter_delay;
    async.enqueue_and_log( "NOTICE", function, message.str() );

    return NO_ERROR;
  }
  /***** Camera::Interface::set_shutter_delay *********************************/


  /***** Camera::Interface::basename ******************************************/
  /**
   * @brief      set or get the image basename
   * @param[in]  args       requested base name
   * @param[out] retstring  current base name
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::basename( const std::string args, std::string &retstring ) {
    const std::string function("Camera::Interface::basename");
    long error=NO_ERROR;

    // Help
    //
    if (args=="?" || args=="help") {
      retstring = CAMERAD_BASENAME;
      retstring.append( " [ <name> ]\n" );
      retstring.append( "  set or get image basename\n" );
      return HELP;
    }

    // Base name cannot contain a "/" because that would be a subdirectory,
    // and subdirectories are not checked here, only by imdir command.
    //
    if ( args.find('/') != std::string::npos ) {
      logwrite( function, "ERROR basename cannot contain '/' character" );
      error = ERROR;
    }
    // if name is supplied the set the image name
    else if ( !args.empty() ) {
      this->camera_info.base_name = args;
      error = NO_ERROR;
    }

    // In any case, log and return the current value.
    //
    logwrite(function, "base name is "+std::string(this->camera_info.base_name));
    retstring = this->camera_info.base_name;

    return error;
  }
  /***** Camera::Interface::basename ******************************************/

}
