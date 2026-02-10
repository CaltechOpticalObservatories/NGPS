/**
 * @file    motion_interface_impl.h
 * @brief   implementations for MotionController::Interface template class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

namespace MotionController {

  /***** MotionController::Interface::has_posname *****************************/
  template <typename ControllerType>
  bool Interface<ControllerType>::has_posname(const std::string &name, const std::string &posname) const {
    if (posname.empty()) return false;
    auto mot = this->motormap.find(name);
    return mot->second.posmap.find(posname) != mot->second.posmap.end();
  }
  /***** MotionController::Interface::has_posname *****************************/


  /***** MotionController::Interface::get_posnames ****************************/
  template <typename ControllerType>
  std::vector<std::string> Interface<ControllerType>::get_posnames(const std::string &name) const {
    std::vector<std::string> keys;
    auto mot = this->motormap.find(name);
    if (mot == this->motormap.end()) return keys;

    for (const auto &pos : mot->second.posmap) {
      keys.push_back(pos.first);
    }
    return keys;
  }
  /***** MotionController::Interface::get_posnames ****************************/


  /***** MotionController::Interface::get_axes ********************************/
  template <typename ControllerType>
  std::vector<int> Interface<ControllerType>::get_axes(const std::string &name) const {
    std::vector<int> keys;
    auto mot = this->motormap.find(name);
    if (mot == this->motormap.end()) return keys;

    for (auto &ax : mot->second.axes) {
      keys.push_back(ax.first);
    }
    return keys;
  }
  /***** MotionController::Interface::get_axes ********************************/


  /***** MotionController::Interface::get_axis ********************************/
  template <typename ControllerType>
  const AxisInfo* Interface<ControllerType>::get_axisinfo(const std::string &name, int axis) const {
    auto mot = this->motormap.find(name);
    if (mot == this->motormap.end()) return nullptr;

    auto ax = mot->second.axes.find(axis);
    if (ax == mot->second.axes.end()) return nullptr;

    return &ax->second;
  }
  /***** MotionController::Interface::get_axis ********************************/


  template <typename ControllerType>
  const std::map<int, AxisInfo>* Interface<ControllerType>::get_axesmap(const std::string &name) const {
    auto it = this->motormap.find(name);
    return ( it != this->motormap.end()) ? &(it->second.axes) : nullptr;
  }


  /***** MotionController::Interface::get_posmap ******************************/
  template <typename ControllerType>
  const std::map<std::string, PosInfo>* Interface<ControllerType>::get_posmap(const std::string &name) const {
    auto it = this->motormap.find(name);
    return ( it != this->motormap.end()) ? &(it->second.posmap) : nullptr;
  }
  template <typename ControllerType>
  const PosInfo* Interface<ControllerType>::get_posinfo(const std::string &name, const std::string &posname) const {
    auto mot = this->motormap.find(name);
    if (mot == this->motormap.end()) return nullptr;

    auto pos = mot->second.posmap.find(posname);
    if (pos == mot->second.posmap.end()) return nullptr;

    return &pos->second;
  }
  /***** MotionController::Interface::get_posmap ******************************/


  /***** MotionController::Interface::get_host ********************************/
  template <typename ControllerType>
  std::string Interface<ControllerType>::get_host(const std::string &name) const {
    auto it = this->motormap.find(name);
    return (it != this->motormap.end()) ? it->second.host : "";
  }
  /***** MotionController::Interface::get_host ********************************/


  /***** MotionController::Interface::get_port ********************************/
  template <typename ControllerType>
  int Interface<ControllerType>::get_port(const std::string &name) const {
    auto it = this->motormap.find(name);
    return (it != this->motormap.end()) ? it->second.port : -1;
  }
  /***** MotionController::Interface::get_port ********************************/


  /***** MotionController::Interface::get_addr ********************************/
  template <typename ControllerType>
  int Interface<ControllerType>::get_addr(const std::string &name) const {
    auto it = this->motormap.find(name);
    return (it != this->motormap.end()) ? it->second.addr : -1;
  }
  /***** MotionController::Interface::get_addr ********************************/


  /***** MotionController::Interface::get_naxes *******************************/
  template <typename ControllerType>
  int Interface<ControllerType>::get_naxes(const std::string &name) const {
    auto it = this->motormap.find(name);
    return (it != this->motormap.end()) ? it->second.naxes : -1;
  }
  /***** MotionController::Interface::get_naxes *******************************/


  /***** MotionController::Interface::open ************************************/
  /**
   * @brief      open a connection to all controllers
   * @details    this is the outside-callable function for open
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::open(const std::string &name) {
    return this->_open(name);
  }
  template <typename ControllerType>
  long Interface<ControllerType>::open() {
    long error=NO_ERROR;
    // loop through motoromap, opening each motor if not already open
    // store a collective error so any one failure returns an error
    //
    for ( const auto &pair : this->motormap ) {
      const std::string &motorname = pair.first;
      error |= this->_open( motorname );
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


  template <typename ControllerType>
  long Interface<ControllerType>::home(const std::string &name, std::string* retstring) {
    const std::string function("MotionController::Interface::home");
    std::vector<std::string> name_list;

    // if input is empty the build a list of all motor names
    if (name.empty()) {
      for ( const auto &mot : this->motormap ) { name_list.push_back(mot.first); }
    }
    else name_list.push_back(name);

    // initialize the thread_error state.
    // threads can modify this atomically to indicate they had an error.
    //
    this->thread_error.store(NO_ERROR);

    // loop through the list of names
    //
    for (const auto &thisname : name_list) {
      auto name_found = this->motormap.find(thisname);

      if (name_found == this->motormap.end()) {
        logwrite(function, "ERROR '"+thisname+"' not found in configuration");
        if (retstring) *retstring="unknown_motor";
        return ERROR;
      }
    }
    return NO_ERROR;
  }

  /***** MotionController::Interface::is_connected ****************************/
  /**
   * @brief      is the socket connected for the specified motor controller?
   * @param[in]  motorname  name of motor controller
   * @return     true or false
   *
   */
  template <typename ControllerType>
  bool Interface<ControllerType>::is_connected(const std::string &motorname) {
    try {
      // get a copy of the socket object for this motor and return its connected status
      //
      Network::TcpSocket &socket = this->get_socket( motorname );
      return ( socket.isconnected() ? true : false );
    }
    catch ( std::runtime_error &e ) {
      std::string function = "MotionController::Interface::is_connected";
      std::stringstream message;
      message.str(""); message << "ERROR: " << e.what();
      logwrite( function, message.str() );
      return false;
    }
  }
  /***** MotionController::Interface::is_connected ****************************/


  /***** MotionController::Interface::add_posmap ******************************/
  /**
   * @brief      add the supplied posinfo to the motormap's posmap
   * @param[in]  posinfo  reference to PosInfo object
   * @return     ERROR or NO_ERROR
   *
   * This function is called whenever the MOTOR_POS key is found
   * in the configuration file. A temporary PosInfo object can be
   * created and loaded with the PosInfo::load_pos_info() function,
   * and then that object can be passed here to store it in
   * the controller class.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::add_posmap(const std::string &name, const PosInfo &posinfo) {
    const std::string function("MotionController::Interface::add_posmap");
    auto it = motormap.find( name );
    if ( it != motormap.end() ) {
      it->second.posmap[posinfo.posname] = posinfo;  // posmap indexed by position name
    }
    else {
      logwrite(function, "ERROR '"+name+"' not found in configuration");
      return ERROR;
    }
    return NO_ERROR;
  }
  /***** MotionController::Interface::add_posmap ******************************/


  /***** MotionController::Interface::add_axis ********************************/
  /**
   * @brief      add the supplied axis info to the motormap's axes vector
   * @param[in]  axis  reference to AxisInfo object
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::add_axis(const std::string &name, const AxisInfo &axis) {
    const std::string function("MotionController::Interface::add_axis");
    // Make sure the motor associated with this axis is already defined
    // in the motormap.
    //
    auto it = motormap.find(name);

    // If it is, then push it onto the axes vector for this motor.
    //
    if ( it != motormap.end() ) {
      it->second.axes[axis.axisnum] = axis;
    }
    else {
      logwrite(function, "ERROR '"+name+"' not found in configuration");
      return ERROR;
    }
    return NO_ERROR;
  }
  /***** MotionController::Interface::add_axis ********************************/


  /***** MotionController::Interface::load_controller_config ******************/
  /**
   * @brief      loads information from the config file into the class
   * @param[in]  input  config file line (the args, not the key)
   * @return     ERROR or NO_ERROR
   *
   * Expected format of input string is:
   * "<motorname> <host> <port> <addr> <naxes>"
   */
  template <typename ControllerType>
  long Interface<ControllerType>::load_controller_config( const std::string &input ) {
    std::string function = "MotionController::Interface::load_controller_config";

    std::istringstream iss(input);
    std::string tryname, tryhost;
    int tryport, tryaddr, trynaxes;

    if (!(iss >> tryname
              >> tryhost
              >> tryport
              >> tryaddr
              >> trynaxes)) {
      logwrite(function, "ERROR: bad config input. Expected { <motorname> <host> <port> <addr> <naxes> }" );
      return ERROR;
    }

    // Create a local (temporary) ControllerInfo object for this ControllerType,
    //
    ControllerInfo<ControllerType> controller;

    // and initialize it with the values just parsed from the config line.
    //
    controller.name  = tryname;
    controller.host  = tryhost;
    controller.port  = tryport;
    controller.addr  = tryaddr;
    controller.naxes = trynaxes;

    // Copy this to the motormap, indexed by motorname.
    //
    this->motormap[tryname] = controller;

    return NO_ERROR;
  }
  /***** MotionController::Interface::load_controller_config ******************/


  /***** MotionController::Interface::get_socket ******************************/
  /**
   * @brief      return a copy of the socket object for the specified motor
   * @details    This can also be used to create a new entry in the socketmap
   *             because if a socket object doesn't exist for the {host,port}
   *             then it will be created and added to the socketmap.
   * @param[in]  motorname  name of motor controller
   * @return     reference to Network::TcpSocket object
   *
   * On error, this function throws std::runtime_error exception; it does
   * not return an error, so it should be used in a try/catch block. It
   * fails only if the requested motorname is not in the motormap.
   *
   */
  template <typename ControllerType>
  Network::TcpSocket& Interface<ControllerType>::get_socket(const std::string &motorname) {
    std::string function = "MotionController::Interface::get_socket";
    std::stringstream message;

    // Find the ControllerInfo for the requested motorname
    //
    auto it = this->motormap.find( motorname );

    if ( it == this->motormap.end() ) {
      message.str(""); message << "motorname \"" << motorname << "\" not found (get_socket)";
      throw std::runtime_error( message.str() );
    }

    // Get the host:port for this motor
    //
    const std::string &host = it->second.host;
    int port = it->second.port;

    // Use this host:port as a key to index the socketmap
    //
    std::pair<std::string, int> key = { host, port };

    // Find the Network::TcpSocket for this motor
    //
    auto socket_it = this->socketmap.find( key );

    // If socket not found in map then create it and add to the socketmap
    //
    if ( socket_it == this->socketmap.end() ) {
      message.str(""); message << "socket not found for motor " << motorname
                               << " on " << host << ":" << port << " (get_socket)";
      Network::TcpSocket new_socket( host, port );
      this->socketmap[key] = new_socket;
      socket_it = this->socketmap.find( key );  // update the iterator
    }

    return socket_it->second;
  }
  /***** MotionController::Interface::get_socket ******************************/

}
