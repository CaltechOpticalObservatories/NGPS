/**
 * @file    motion_controller_impl.h
 * @brief   implementations for MotionController::ControllerInfo template class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

namespace MotionController {


  /***** MotionController::Interface::send_command ****************************/
  /**
   * @brief      send specified command to socket interface and optionally return reply
   * @details    This function will expect and return a reply if the retstring
   *             parameter is provided (defaults to nullptr).
   * @param[in]  name       name of motor
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply read back from device if provided
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::send_command(const std::string &name, const std::string &cmd, std::string* retstring) {
    std::string function = "MotionController::Interface::send_command";
    std::ostringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    Network::TcpSocket* socket=nullptr;

    std::unique_lock<std::mutex> lock( *this->controller_mutex );

    try {
      socket = &this->get_socket(name);   // includes check of motorname
    }
    catch (const std::runtime_error &e) {
      logwrite(function, "ERROR: "+std::string(e.what()));
      return ERROR;
    }

    if ( !socket->isconnected() ) {
      logwrite(function, "ERROR no socket connection to motor "+name);
      return ERROR;
    }

    int written = socket->Write( cmd+"\n" );  // write the command with newline character

    if ( written <= 0 ) return ERROR;         // return error if error writing to socket

    if (retstring==nullptr) return NO_ERROR;  // no reply needed

    // read the reply when retstring is supplied
    //
    while ( error == NO_ERROR && retval >= 0 ) {

      if ( ( retval=socket->Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "TIMEOUT on fd " << socket->getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "ERROR on fd " << socket->getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = socket->Read( reply, '\n' ) ) < 0 ) {
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
  /***** MotionController::Interface::send_command ****************************/


  /***** ControllerInfo::load_controller_info *********************************/
  /**
   * @brief      Loads information from the config file into the class
   * @details    This is the template class version which can parse the
   *             input string for common parameters, then call the
   *             specialized version which will extract parameters specific
   *             to that type of controller. OBSOLETE
   * @param[in]  input  string containing line from config file
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename T>
  long ControllerInfo<T>::load_controller_info( std::string input ) {
    return T::load_controller_info( input );
  }
  /***** ControllerInfo::load_controller_info *********************************/


  /***** ControllerInfo::load_pos_info ****************************************/
  /**
   * @brief      Loads MOTOR_POS information from config file into the 
   * @details    This is the template class version which will parse the
   *             input string for common parameters.
   * @param[in]  input  string specifies: "<name> <ID> <pos> <posname>"
   * @return     ERROR or NO_ERROR
   *
   * This function is called whenever the MOTOR_POS key is found
   * in the configuration file, to parse and load all of the information
   * assigned by that key into the appropriate class variables.
   *
   * Currently all motors use the same MOTOR_POS format, but if that changes
   * then a template class function call could be made here.
   *
   */
  template <typename T>
  long ControllerInfo<T>::load_pos_info( std::string input ) {
    std::string function = "MotionController::ControllerInfo::load_pos_info";
    int id;
    float position;
    std::string name, posname;

    std::istringstream iss(input);

    if (!(iss >> name
              >> id
              >> position
              >> posname)) {
      logwrite(function, "ERROR bad config input. expected { <name> <ID> <pos> <posname> }");
      return ERROR;
    }

    // ID starts at 0 (can't be negative)
    //
    if ( id < 0 ) {
      std::ostringstream oss;
      oss << "ERROR: id " << id << " for \"" << name << "\" " << "must be >= 0";
      logwrite(function, oss.str());
      return ERROR;
    }

    this->name = name;
    this->posmap[ posname ].id       = id;
    this->posmap[ posname ].position = position;
    this->posmap[ posname ].posname  = posname;

    return NO_ERROR;
  }
  /***** ControllerInfo::load_pos_info ****************************************/

}

