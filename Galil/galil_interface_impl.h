/**
 * @file    galil_interface_impl.h
 * @brief   this file contains the implementation for the Galil Interface class
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */


namespace Galil {

  /***** Galil::Interface::test ***********************************************/
  template <typename Type>
  void Interface<Type>::test() {
  }
  /***** Galil::Interface::test ***********************************************/


  /***** Galil::Interface::stop ***********************************************/
  template <typename Type>
  long Interface<Type>::stop(const std::string &name) {
    logwrite("Galil::Interface::stop", "not implemented");
    return ERROR;
  }
  /***** Galil::Interface::stop ***********************************************/


  /***** Galil::Interface::is_home ********************************************/
  template <typename Type>
  bool Interface<Type>::is_home(const std::string &name) {
    throw std::runtime_error("not implemented");
  }
  /***** Galil::Interface::is_home ********************************************/


  /***** Galil::Interface::moveto *********************************************/
  template <typename Type>
  long Interface<Type>::moveto(const std::string &name,
                               int axisnum,
                               const std::string &posstr,
                               std::string &retstring) {
    const std::string function("Galil::Interface::moveto");
    logwrite(function, "ERROR not implemented");
    return ERROR;
  }
  /***** Galil::Interface::moveto *********************************************/


  /***** Galil::Interface::get_pos ********************************************/
  /**
   * @brief      get the current position of a motor
   * @details    This is the outside-callable function for reading a position,
   *             which performs all the safety checks on name, axis, etc.
   * @param[in]  name      controller name
   * @param[in]  axisnum   axis number
   * @param[out] position  reference to position read
   * @param[out] posname   optional reference to position name, if one exists for pos
   * @param[in]  addr      optional address of controller in daisy-chain
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::get_pos(const std::string &name, int axisnum, float &position,
                                          std::string* posname, int addr) {
    const std::string function("Galil::Interface::get_pos");
    logwrite(function, "ERROR not implemented");
    return ERROR;
  }
  /***** Galil::Interface::get_pos ********************************************/


  /***** Galil::Interface::move_to_default ************************************/
  /**
   * @brief      move to the default position
   * @details    this is the outside-callable function
   * @return     ERROR | NO_ERROR
   *
   * Move all motor axes to their defaults, if specified.
   *
   */
  template <typename Type>
  long Interface<Type>::move_to_default() {
    logwrite("Galil::Interface::move_to_default", "not yet implemented");
    return ERROR;
  }
  /***** Galil::Interface::move_to_default ************************************/


  /***** Galil::Interface::send_command ***************************************/
  /**
   * @brief      send specified command to Galil socket interface and return reply
   * @details    This function will expect and return a reply if the retstring
   *             parameter is provided (defaults to nullptr).
   * @param[in]  motorname  name of motor
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply read back from device if provided
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename Type>
  long Interface<Type>::Xend_command(const std::string &motorname, std::string cmd, std::string* retstring) {
    std::string function = "Galil::Interface::send_command";
    std::ostringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    Network::TcpSocket socket;

    std::unique_lock<std::mutex> lock( *this->controller_mutex );

    try {
      socket = this->get_socket( motorname );   // includes check of motorname
    }
    catch ( const std::runtime_error &e ) {
      logwrite(function, "ERROR: "+std::string(e.what()));
      return ERROR;
    }

    if ( !socket.isconnected() ) {
      logwrite(function, "ERROR no socket connection to motor "+motorname);
      return ERROR;
    }

    cmd.append( "\n" );                       // add the newline character

    int written = socket.Write( cmd );        // write the command

    if ( written <= 0 ) return ERROR;         // return error if error writing to socket

    if (retstring==nullptr) return NO_ERROR;  // no reply needed

    // read the reply
    //
    while ( error == NO_ERROR && retval >= 0 ) {

      if ( ( retval=socket.Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "TIMEOUT on fd " << socket.getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "ERROR on fd " << socket.getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = socket.Read( reply, '\n' ) ) < 0 ) {
        message.str(""); message << "ERROR reading from motor controller: " << strerror( errno );
        logwrite( function, message.str() );
        break;
      }

      // remove any newline characters and get out
      //
      strip_newline(reply);
      break;
    }

    *retstring = reply;

    return error;
  }
  /***** Galil::Interface::send_command ***************************************/

}
