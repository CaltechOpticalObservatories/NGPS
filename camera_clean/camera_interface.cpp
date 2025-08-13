#include "camera_interface.h"

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
