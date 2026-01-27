/**
 * @file    motion_controller_impl.h
 * @brief   this file contains implementations for template class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

namespace MotionController {

  /***** MotionController::Interface::open ************************************/
  /**
   * @brief      open a connection to all controllers
   * @details    this is the outside-callable function for open
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::open() {
    long error=NO_ERROR;
    // loop through motoromap, opening each motor if not already open
    // store a collective error so any one failure returns an error
    //
    for ( const auto &pair : this->motormap ) {
      const std::string &motorname = pair.first;
      if ( !is_connected( motorname ) ) { error |= this->_open( motorname ); }
    }
    return error;
  }
  /***** MotionController::Interface::open ************************************/


  /***** MotionController::Interface::_open ***********************************/
  /**
   * @brief      open a connection to the specified controller (private)
   * @param[in]  motorname  reference to motor to find in map
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_open( const std::string &motorname ) {
    std::string function = "MotionController::Interface::_open";
    std::ostringstream message;

    try {
      Network::TcpSocket &socket = this->get_socket( motorname );

      if ( socket.isconnected() ) {
        message.str(""); message << "connection open for " << motorname
                                 << " on " << socket.gethost() << ":" << socket.getport()
                                 << " with fd " << socket.getfd();
        logwrite( function, message.str() );
        return NO_ERROR;
      }

      if ( socket.Connect() != 0 ) {
        message.str(""); message << "ERROR connecting to " << motorname
                                 << " on " << socket.gethost() << ":" << socket.getport();
        logwrite( function, message.str() );
        return ERROR;
      }

      message.str(""); message << "connection to " << motorname
                               << " established on host " << socket.gethost() << ":" << socket.getport()
                               << " with fd " << socket.getfd();
      logwrite( function, message.str() );
    }
    catch ( std::runtime_error &e ) {
      logwrite(function, "ERROR: "+std::string(e.what()));
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** MotionController::Interface::_open ***********************************/


  /***** MotionController::Interface::close ***********************************/
  /**
   * @brief      close the connection to all controllers
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::close() {
    std::string function = "MotionController::Interface::close";
    long error=NO_ERROR;
    for ( const auto &pair : this->motormap ) {
      const std::string &motorname = pair.first;
      error |= this->close( motorname );
    }
    return error;
  }
  /***** MotionController::Interface::close ***********************************/
  /**
   * @brief      close the connection to all controllers
   * @param[in]  motorname  reference to motor name to close
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::close(const std::string &motorname) {
    std::string function = "MotionController::Interface::close";

    try {
      Network::TcpSocket &socket = this->get_socket( motorname );

      if ( !socket.isconnected() ) return NO_ERROR;

      long error = socket.Close();

      if ( error == NO_ERROR ) {
        logwrite(function, "socket connection to "+motorname+" closed");
      }
      else {
        std::ostringstream message;
        message << "ERROR close socket connection to " << motorname
                << " on host " << socket.gethost() << ":" << socket.getport();
        logwrite(function, message.str());
      }
      return error;
    }
    catch ( std::runtime_error &e ) {
      logwrite(function, "ERROR: "+std::string(e.what()));
      return ERROR;
    }
  }
  /***** MotionController::Interface::close ***********************************/

}
