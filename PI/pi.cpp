/**
 * @file    pi.cpp
 * @brief   this file contains the code for the PI interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "pi.h"
#include "logentry.h"

namespace Physik_Instrumente {

  template <typename ControllerType>
  void Interface<ControllerType>::test() {
  }


  template <typename ControllerType>
  bool Interface<ControllerType>::is_home(const std::string &name) {
    if (this->motormap.find(name) == this->motormap.end()) {
      throw std::runtime_error("'"+name+"' not configured");
    }
    if (!this->is_connected(name)) {
      throw std::runtime_error("'"+name+"' not connected");
    }

    auto addr = this->motormap.at(name).addr;
    int axis = 1;
    bool _ishome;

    if (this->is_home( name, addr, axis, _ishome ) != NO_ERROR) {
      throw std::runtime_error("'"+name+"' communication error");
    }

    return _ishome;
  }


  /***** Physik_Instrumente::Interface::enable_motion *************************/
  /**
   * @brief      enable/disable motion
   * @details    This sets the servo on/off
   * @param[in]  shouldenable  true|false
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::enable_motion(bool shouldenable) {
    return this->set_servo(shouldenable);
  }
  /***** Physik_Instrumente::Interface::enable_motion *************************/


  /***** Physik_Instrumente::Interface::stop **********************************/
  /**
   * @brief      enable/disable motion
   * @details    This sets the servo on/off
   * @param[in]  shouldenable  true|false
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::stop(const std::string &name) {
    return this->stop_motion(name, -1);
  }
  /***** Physik_Instrumente::Interface::stop **********************************/


  /***** Physik_Instrumente::Interface::enable_motion *************************/
  /**
   * @brief      enable/disable motion for specified name
   * @details    This sets the servo on/off
   * @param[in]  name          motor name
   * @param[in]  shouldenable  true|false
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::enable_motion(const std::string &name, bool shouldenable) {
    return this->set_servo(name, shouldenable);
  }
  /***** Physik_Instrumente::Interface::enable_motion *************************/


  /***** Physik_Instrumente::Interface::clear_errors **************************/
  /**
   * @brief      clear error status of all controllers
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::clear_errors( ) {
    long error = NO_ERROR;

    for ( const auto &pair : this->motormap ) {
      const std::string &motorname = pair.first;
      int addr = pair.second.addr;
      int dontcare;

      error |= this->get_error( motorname, addr, dontcare );  // reading clears it
    }

    return error;
  }
  /***** Physik_Instrumente::Interface::clear_errors **************************/


  /***** Physik_Instrumente::Interface::get_error *****************************/
  /**
   * @brief      read the error status for the specified controller
   * @param[in]  name     name of motor controller
   * @param[in]  addr     address of controller
   * @param[out] errcode  reference to int to return error code
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::get_error( const std::string &name, int addr, int &errcode ) {
    std::string function = "Physik_Instrumente::Interface::get_error";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "ERR?";

    long error = this->send_command(name, cmd.str(), &reply);                // send the command

    if ( error != NO_ERROR ) return error;

    // When an addr is specified (>0) then the response will be 0 A R
    // where A is addr and R is the response. Grab the last token string here...
    //
    if ( addr > 0 ) {
      std::vector<std::string> tokens;
      Tokenize( reply, tokens, " " );
      if ( tokens.size() != 3 ) {
        errcode = -1;
        message.str(""); message << "ERROR bad reply \"" << reply << "\": expected 3 tokens";
        logwrite( function, message.str() );
        return ERROR;
      }
      try {
        reply = tokens.at(2);                    // set the reply string to the last token
      }
      catch ( std::out_of_range & ) {
        errcode = -1;
        logwrite( function, "ERROR reply token out of range" );
        return ERROR;
      }
    }

    // Now, reply has either been over-written above (for addr>0)
    // or in the case where addr was not specified (-1) then then response will be simply, R
    // which is stored in reply. In either case, convert reply to <int> and that is the errcode.
    //
    try {
      errcode = std::stoi( reply );
    }
    catch ( std::invalid_argument &e ) {
      errcode = -1;
      message.str(""); message << "ERROR bad reply \"" << reply << "\": unable to convert to integer: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch ( std::out_of_range & ) {
      errcode = -1;
      logwrite( function, "ERROR reply out of integer range" );
      return ERROR;
    }

    return error;
  }
  /***** Physik_Instrumente::Interface::get_error *****************************/


  /***** Physik_Instrumente::Interface::set_servo *****************************/
  /**
   * @brief      set all servos on|off
   * @details    loop through all axes of all motors
   * @param[in]  state  (true=on, false=off)
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::set_servo( bool state ) {
    long error = NO_ERROR;

    for ( const auto &pair : this->motormap ) {
      const std::string &motorname = pair.first;
      int addr = pair.second.addr;
      for ( const auto &axes : pair.second.axes ) {
        int axis = axes.second.axisnum;
        error |= this->set_servo( motorname, addr, axis, state );
      }
    }

    return error;
  }
  /***** Physik_Instrumente::Interface::set_servo *****************************/


  /***** Physik_Instrumente::Interface::set_servo *****************************/
  /**
   * @brief      set servos on|off for specified name
   * @param[in]  name   motor name
   * @param[in]  state  (true=on, false=off)
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::set_servo(const std::string &name, bool state) {
    int addr = this->get_addr(name);
    return this->set_servo(name, addr, -1, state);
  }
  /***** Physik_Instrumente::Interface::set_servo *****************************/


  /***** Physik_Instrumente::Interface::set_servo *****************************/
  /**
   * @brief      set the specified servo on|off, all axes
   * @param[in]  name   controller name
   * @param[in]  addr   address of controller
   * @param[in]  state  (true=on, false=off)
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::set_servo( const std::string &name, int addr, bool state ) {
    return( this->set_servo( name, addr, -1, state ) );
  }
  /***** Physik_Instrumente::Interface::set_servo *****************************/


  /***** Physik_Instrumente::Interface::set_servo *****************************/
  /**
   * @brief      set the specified axis for specified servo on|off
   * @param[in]  name   controller name
   * @param[in]  addr   address of controller
   * @param[in]  axis   axis
   * @param[in]  state  (true=on, false=off)
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::set_servo( const std::string &name, int addr, int axis, bool state ) {
    std::stringstream cmd;
    if ( addr > 0 ) cmd << addr << " ";
    cmd << "SVO";
    if ( axis > 0 ) cmd << " " << axis;
    cmd << " " << ( state ? 1 : 0 );
    this->send_command( name, cmd.str() );
    return NO_ERROR;
  }
  /***** Physik_Instrumente::Interface::set_servo *****************************/


  /***** Physik_Instrumente::Interface::moveto ********************************/
  /**
   * @brief      move multiple axes (absolute) to requested positions
   * @details    this is the outside-callable function for moving multiple actuators
   * @param[in]  motornames  vector of motor controller names
   * @param[in]  axisnums    vector of axis numbers
   * @param[in]  posnames    vector of position names (or position values, see below)
   * @param[out] retstring  reference to return string
   * @return     ERROR or NO_ERROR
   *
   * All actuators in the vector will be moved simultaneously in separate threads.
   * This function will wait for all moves to complete and return only after all
   * have moved (or timed out).
   *
   * This function is overloaded
   *
   * The posnames vector can also be a string representation of a float position.
   * If the posname is not found in the posmap then this function will try to
   * convert it to a float to use as a position value.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::moveto( std::vector<std::string> motornames,
                                          std::vector<int> axisnums,
                                          std::vector<std::string> posnames,
                                          std::string &retstring ) {
    std::string function = "Physik_Instrumente::Interface::moveto";
    std::stringstream message;

    // check for equal vector sizes at least
    //
    if ( ! ( motornames.size() == posnames.size() && posnames.size() == axisnums.size() ) ) {
      message.str(""); message << "ERROR mismatch: number of actuators " << motornames.size()
                               << " must equal number of positions " << posnames.size()
                               << " and axes " << axisnums.size();
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    // Check that requested motornames are defined in the motormap,
    // and that they are connected.
    //
    for ( const auto &name : motornames ) {
      auto name_found = this->motormap.find( name );
      if ( name_found == this->motormap.end() ) {
        message.str(""); message << "ERROR actuator \"" << name << "\" not found in motormap: {";
        for ( const auto &mot : this->motormap ) message << " " << mot.first;
        message << " }";
        logwrite( function, message.str() );
        retstring="unknown_motor";
        return ERROR;
      }

      // Even though _dothread_moveto checks this (it must, for safety, in case it's used elsewhere),
      // check connection status here so that the caller gets the connection error in the retstring.
      //
      if ( ! this->is_connected( name ) ) {
        message.str(""); message << "ERROR not connected to motor controller " << name;
        logwrite( function, message.str() );
        retstring="not_connected";
        return ERROR;
      }
    }

    // Loop through the vectors (they're all the same size) and
    // check that the requested posnames are defined in the posmaps for the motors,
    // and retrieve the positions associated with those posnames. If the posname
    // isn't found then try to convert it to a float (maybe we were given a position).
    //
    std::vector<float> positions;
    std::vector<int> addrs;
    for ( size_t n=0; n < motornames.size(); n++ ) {

      auto motorname = motornames[n];
      auto posname   = posnames[n];
      auto pos_found = this->motormap[motorname].posmap.find( posname );

      if ( pos_found != this->motormap[motorname].posmap.end() ) {
        positions.push_back( this->motormap[motorname].posmap[posname].position );
        addrs.push_back( this->motormap[motorname].addr );
      }
      else {
        // posname not in posmap.
        // maybe we were given a string representation of a number?
        // If conversion to float is successful then it's a number.
        //
        bool is_number;
        try {
          positions.push_back( std::stof( posname ) );
          addrs.push_back( this->motormap[motorname].addr );
          is_number=true;
        }
        catch( const std::invalid_argument &e ) { is_number=false; }
        catch( const std::out_of_range &e )     { is_number=false; }

        // Conversion to float did not succeed so now report the error.
        //
        if ( !is_number ) {
          message.str(""); message << "ERROR position \"" << posname << "\" not in list {";
          for ( const auto &pos : this->motormap[motorname].posmap ) message << " " << pos.first;
          message << " } for actuator " << motorname << " and can't be converted to a float";
          logwrite( function, message.str() );
          retstring="invalid_position";
          return ERROR;
        }
      }
    }

    // Now we have validated vectors of motornames, addrs, axes and positions,
    // so spawn threads to move them all.
    //

    this->thread_error.store( NO_ERROR );  // initialize the thread_error state.

    for ( size_t n=0; n < motornames.size(); n++ ) {

      auto name = motornames[n];
      auto addr = addrs[n];
      auto axis = axisnums[n];
      auto position = positions[n];

      // Spawn a thread to performm the move.
      // If there is more than one then they can be done in parallel.
      //
      std::thread( _dothread_moveto, std::ref( *this ), name, addr, axis, position ).detach();
      this->motors_running++;
    }

    // wait for the threads to finish
    // TODO add a way to abort this
    //
    while ( this->motors_running != 0 ) {
      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    logwrite( function, "move(s) complete" );

    return( this->thread_error.load() );   // return any errors from the threads
  }
  /***** Physik_Instrumente::Interface::moveto ********************************/


  /***** Physik_Instrumente::Interface::moveto ********************************/
  /**
   * @brief      move an axis (absolute) to requested position name
   * @details    This is the outside-callable function for moving.
   *             Can be used to move to a position name or a numeric
   *             position. If the posname isn't recognized then will try
   *             to convert the value to a float.
   * @param[in]  name       motor controller name
   * @param[in]  axisnum    axis number
   * @param[in]  posname    position name (or number, see details)
   * @param[out] retstring  reference to return string
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::moveto(const std::string &name, int axisnum, const std::string &posstr, std::string &retstring) {
    const std::string function("Physik_Instrumente::Interface::moveto");
    std::stringstream message;

    // Check that requested name is defined in the motormap
    //
    auto name_found = this->motormap.find( name );
    if ( name_found == this->motormap.end() ) {
      message.str(""); message << "ERROR actuator \"" << name << "\" not found in motormap: {";
      for ( const auto &mot : this->motormap ) message << " " << mot.first;
      message << " }";
      logwrite( function, message.str() );
      retstring="unknown_motor";
      return ERROR;
    }

    // If the requested posstr is defined in the posmap for this motor, then
    // call the moveto() function with the position.
    //
    float position;
    auto pos_found = this->motormap[name].posmap.find( posstr );
    if ( pos_found != this->motormap[name].posmap.end() ) {
      position = this->motormap[name].posmap[posstr].position;
    }
    else {
      // posstr not in posmap.
      // maybe we were given a string representation of a number?
      //
      bool is_number;
      try {
        position = std::stof( posstr );
        is_number=true;
      }
      catch( const std::invalid_argument &e ) { is_number=false; }
      catch( const std::out_of_range &e )     { is_number=false; }

      if ( !is_number ) {
        message.str(""); message << "ERROR position \"" << posstr << "\" not in list {";
        for ( const auto &pos : this->motormap[name].posmap ) message << " " << pos.first;
        message << " } for actuator " << name << " and can't be converted to a float";
        logwrite( function, message.str() );
        retstring="invalid_position";
        return ERROR;
      }
    }

    return this->moveto( name, axisnum, position, retstring );
  }
  /***** Physik_Instrumente::Interface::moveto ********************************/


  /***** Physik_Instrumente::Interface::moveto ********************************/
  /**
   * @brief      move an axis (absolute) to requested position value
   * @details    this is the outside-callable function for moving
   * @param[in]  motorname  motor controller name
   * @param[in]  axisnum    axis number
   * @param[in]  position   position value
   * @param[out] retstring  reference to return string
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::moveto( std::string motorname, int axisnum, float position, std::string &retstring ) {
    std::string function = "Physik_Instrumente::Interface::moveto";
    std::stringstream message;

    // Check that requested motorname is defined in the motormap
    //
    auto name_found = this->motormap.find( motorname );
    if ( name_found == this->motormap.end() ) {
      message.str(""); message << "ERROR actuator \"" << motorname << "\" not found in motormap: {";
      for ( const auto &mot : this->motormap ) message << " " << mot.first;
      message << " }";
      logwrite( function, message.str() );
      retstring="unknown_motor";
      return ERROR;
    }
    auto addr = this->motormap[motorname].addr;

    // Get the min/max for this axisnum
    //
    auto axis_found = this->motormap[motorname].axes.find( axisnum );
    if ( axis_found != this->motormap[motorname].axes.end() ) {
      float min = this->motormap[motorname].axes[axisnum].min;
      float max = this->motormap[motorname].axes[axisnum].max;

      if ( position < min || position > max ) {
        message.str(""); message << "ERROR position " << position << " outside range { " << min << " : " << max << " } "
                                 << "for axis " << axisnum << " actuator " << motorname;
        logwrite( function, message.str() );
        retstring="invalid_position";
        return ERROR;
      }
    }
    else {
      message.str(""); message << "ERROR axisnum " << axisnum << " not defined for actuator " << motorname;
      logwrite( function, message.str() );
      retstring="invalid_axis";
      return ERROR;
    }

    // send the move_abs command
    //
    long error = this->_move_abs( motorname, addr, axisnum, position );

    // and wait for the move if successful
    //
    if ( error==NO_ERROR ) error = this->_move_axis_wait( motorname, addr, axisnum );

    return error;
  }
  /***** Physik_Instrumente::Interface::moveto ********************************/


  /***** Physik_Instrumente::Interface::_move_abs *****************************/
  /**
   * @brief      send move command in absolute coordinates to specified motor (private)
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller
   * @param[in]  pos   absolute position to move to
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_move_abs( const std::string &name, int addr, float pos ) {
    return( this->_move_abs( name, addr, -1, pos ) );
  }
  /***** Physik_Instrumente::Interface::_move_abs *****************************/


  /***** Physik_Instrumente::Interface::_move_abs *****************************/
  /**
   * @brief      send move command in absolute coordinates (private)
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller
   * @param[in]  axis  axis to move
   * @param[in]  pos   absolute position to move to
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis is not specified.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_move_abs( const std::string &name, int addr, int axis, float pos ) {
    std::string function = "Physik_Instrumente::Interface::_move_abs";
    std::stringstream message;
    std::stringstream cmd;
    if ( addr < 0 ) {
      message.str(""); message << "ERROR: bad address " << addr;
      logwrite( function, message.str() );
      return ERROR;
    }
    if ( std::isnan( pos ) ) {
      logwrite( function, "ERROR: position is NaN" );
      return ERROR;
    }
    if ( addr > 0 ) cmd << addr << " ";
    cmd << "MOV";
    if ( axis > 0 ) cmd << " " << axis;
    cmd << " " << pos;
    this->send_command( name, cmd.str() );
    return NO_ERROR;
  }
  /***** Physik_Instrumente::Interface::_move_abs *****************************/


  /***** Physik_Instrumente::Interface::_dothread_moveto **********************/
  /**
   * @brief      threaded function to move to an absolute position (private)
   * @details    this is the work function for moveto
   * @param[in]  iface  reference to *this
   * @param[in]  name   name of motor
   * @param[in]  addr   addr of motor
   * @param[in]  axis   axisnum
   * @param[in]  pos    position
   *
   * This is the work function to call Interface::_move_abs() in a thread
   *
   */
  template <typename ControllerType>
  void Interface<ControllerType>::_dothread_moveto( Interface<ControllerType> &iface,
                                                   const std::string name,
                                                   const int addr,
                                                   const int axis,
                                                   const float pos ) {
    std::string function = "Physik_Instrumente::Interface::_dothread_moveto";
    std::stringstream message;
    long error = NO_ERROR;

    // requires an open connection
    //
    if ( ! iface.is_connected( name ) ) {
      message.str(""); message << "ERROR not connected to motor controller " << name;
      logwrite( function, message.str() );
      error = ERROR;
    }
    else {

      // send the move command by calling move_axis()
      //
      error = iface._move_abs( name, addr, axis, pos );

      if ( error == NO_ERROR ) {
        logwrite( function, "waiting for "+name );
        error = iface._move_axis_wait( name, addr, axis );   // this can time out
      }
    }

    iface.thread_error.fetch_or( error );        // preserve any error returned

    --iface.motors_running;                      // atomically decrement the number of motors waiting

    message.str(""); message << "completed move " << name << ( error!=NO_ERROR ? " with error" : "" );
    logwrite( function, message.str() );

    return;
  }
  /***** Physik_Instrumente::Interface::_dothread_moveto **********************/


  /***** Physik_Instrumente::Interface::_move_rel *****************************/
  /**
   * @brief      move in relative coordinates (private)
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller
   * @param[in]  pos   absolute position to move to
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_move_rel( const std::string &name, int addr, float pos ) {
    return( this->_move_rel( name, addr, -1, pos ) );
  }
  /***** Physik_Instrumente::Interface::_move_rel *****************************/


  /***** Physik_Instrumente::Interface::_move_rel *****************************/
  /**
   * @brief      move in relative coordinates (private)
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller
   * @param[in]  axis  axis to move
   * @param[in]  pos   absolute position to move to
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis is not specified.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_move_rel( const std::string &name, int addr, int axis, float pos ) {
    std::string function = "Physik_Instrumente::Interface::_move_rel";
    std::stringstream message;
    std::stringstream cmd;
    if ( addr < 0 ) {
      message.str(""); message << "ERROR: bad address " << addr;
      logwrite( function, message.str() );
      return ERROR;
    }
    if ( std::isnan( pos ) ) {
      logwrite( function, "ERROR: position is NaN" );
      return ERROR;
    }
    if ( addr > 0 ) cmd << addr << " ";
    cmd << "MVR";
    if ( axis > 0 ) cmd << " " << axis;
    cmd << " " << pos;
    this->send_command( name, cmd.str() );
    return NO_ERROR;
  }
  /***** Physik_Instrumente::Interface::_move_rel *****************************/


  /***** Physik_Instrumente::Interface::move_to_default ***********************/
  /**
   * @brief      move to the default position
   * @details    this is the outside-callable function
   * @return     ERROR | NO_ERROR
   *
   * Move all motor axes to their defaults, if specified.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::move_to_default() {

    for ( const auto &mot : this->motormap ) {
      for ( const auto &axis : mot.second.axes ) {
        if ( !std::isnan(axis.second.defpos) ) {
          std::thread( _dothread_moveto, std::ref( *this ), mot.first, mot.second.addr, axis.second.axisnum, axis.second.defpos ).join();
        }
      }
    }

    // get any errors from the threads
    //
    return( this->thread_error.load() );
  }
  /***** Physik_Instrumente::Interface::move_to_default ***********************/


  /***** Physik_Instrumente::Interface::home **********************************/
  /**
   * @brief      home an axis by moving to reference switch (private)
   * @details    this is the outside-callable function for homeing
   * @param[in]  input      may contain list of motor names
   * @param[in]  retstring  reference to return string
   * @return     ERROR or NO_ERROR
   *
   * The input string can contain a space-delimited list of motor names to home,
   * or if empty then all motors will be homed. This function will spawn a
   * separate thread for each motor, then will wait for all threads to complete
   * before returning.
   *
   */
  /***
  template <typename ControllerType>
  long Interface<ControllerType>::home( std::string input, std::string &retstring ) {
    std::string function = "Physik_Instrumente::Interface::home";
    std::stringstream message;
    std::vector<std::string> name_list;

    // If input is empty then build up a vector of each motor name
    //
    if ( input.empty() ) {
      for ( const auto &mot : this->motormap ) { name_list.push_back( mot.first ); }
    }
    else {
      Tokenize( input, name_list, " " );
    }

    // initialize the thread_error state.
    // threads can modify this atomically to indicate they had an error.
    //
    this->thread_error.store( NO_ERROR );

    // Now loop through the built up list of motor names
    //
    for ( const auto &name : name_list ) {

      auto name_found = this->motormap.find( name );

      if ( name_found == this->motormap.end() ) {
        message.str(""); message << "ERROR actuator \"" << name << "\" not found in motormap: {";
        for ( const auto &mot : this->motormap ) message << " " << mot.first;
        message << " }";
        logwrite( function, message.str() );
        retstring="unknown_motor";
        return ERROR;
      }

      // Spawn a thread to performm the home move.
      // If there is more than one then they can be done in parallel.
      //
      std::thread( _dothread_home, std::ref( *this ), name ).detach();
      this->motors_running++;
    }

    // wait for the threads to finish
    // TODO add a way to abort this
    //
    while ( this->motors_running != 0 ) {
      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    logwrite( function, "home complete" );

    // get any errors from the threads
    //
    return( this->thread_error.load() );
  }
  ***/
  /***** Physik_Instrumente::Interface::home **********************************/


  /***** Physik_Instrumente::Interface::_dothread_home ************************/
  /**
   * @brief      threaded function to home a motor (private)
   * @details    this is the work function for homeing
   * @param[in]  iface  reference to *this
   * @param[in]  name   name of motor to home
   *
   * This is the work function to call Interface::home() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by functions
   * called in here are set in the thread_error class variable.
   *
   */
  template <typename ControllerType>
  void Interface<ControllerType>::_dothread_home( Interface<ControllerType> &iface, const std::string name ) {
    std::string function = "Physik_Instrumente::Interface::_dothread_home";
    std::stringstream message;
    std::string reftype;
    int axis=1;  // TODO remove limitation of single axis
    int addr;
    long error = NO_ERROR;

    try {
      addr    = iface.motormap.at(name).addr;
      reftype = iface.motormap.at(name).axes.at(axis).reftype;
    }
    catch ( const std::out_of_range &e ) {
      message.str(""); message << "ERROR: name \"" << name << "\" not in motormap: " << e.what();
      logwrite( function, message.str() );
      iface.thread_error.fetch_or( ERROR );      // preserve this error
      --iface.motors_running;                    // atomically decrement the number of motors waiting
      return;
    }

    if ( reftype.empty() ) {
      message.str(""); message << "NOTICE referencing not available for " << name;
      logwrite( function, message.str() );
      --iface.motors_running;                    // atomically decrement the number of motors waiting
      return;
    }

    // requires an open connection
    //
    if ( ! iface.is_connected( name ) ) {
      message.str(""); message << "ERROR not connected to motor controller " << name;
      logwrite( function, message.str() );
      error = ERROR;
    }
    else {

      // send the home command by calling home_axis()
      //
      error = iface._home_axis( name, addr, axis, reftype );

      if ( error == NO_ERROR ) {
        logwrite( function, "waiting for "+name );
        error = iface._home_axis_wait( name, addr, axis );  // this can time out
      }
    }

//  // If successful, apply the zeropos if necessary
//  //
//  auto zeropos = iface.motormap[name].axes[axis].zeropos;
//  if ( error==NO_ERROR && zeropos != 0 ) {
//    logwrite( function, "applying zeropos offset" );
//    error = iface._move_abs( name, addr, axis, zeropos );                 // move to zeropos position
////  std::stringstream cmd;
////  cmd << addr << " DFH " << axis;
////  if ( error==NO_ERROR ) error = iface.send_command( name, cmd.str() );  // define this as the home position
//    if ( error==NO_ERROR ) {
//      logwrite( function, "waiting for "+name );
//      error = iface._move_axis_wait( name, addr, axis );   // this can time out
//    }
//  }

    // If successful, send to default position, if specified
    //
    auto defpos = iface.motormap[name].axes[axis].defpos;
    if ( error==NO_ERROR && !std::isnan(defpos) ) {
      logwrite( function, "sending to default position" );
      error = iface._move_abs( name, addr, axis, defpos );                  // move to default position
      if ( error==NO_ERROR ) {
        logwrite( function, "waiting for "+name );
        error = iface._move_axis_wait( name, addr, axis );   // this can time out
      }
    }

    iface.thread_error.fetch_or( error );        // preserve any error returned

    --iface.motors_running;                      // atomically decrement the number of motors waiting

    message.str(""); message << "completed home " << name << ( error!=NO_ERROR ? " with error" : "" );
    logwrite( function, message.str() );

    return;
  }
  /***** Physik_Instrumente::Interface::_dothread_home ************************/


  /***** Physik_Instrumente::Interface::_home_axis ****************************/
  /**
   * @brief      home an axis by moving to reference switch (private)
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller in daisy-chain
   * @param[in]  ref   what to use for the homing
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   * Homing can be performed using a reference switch (if equipped), or the 
   * positive or negative limit switches, as indicated by the "ref" argument.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_home_axis( const std::string &name, int addr, std::string ref ) {
    return( this->_home_axis( name, addr, -1, ref ) );       //!< all axes at this addr
  }
  /***** Physik_Instrumente::Interface::_home_axis ****************************/


  /***** Physik_Instrumente::Interface::_home_axis ****************************/
  /**
   * @brief      home an axis by moving to reference switch (private)
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller in daisy-chain
   * @param[in]  axis  axis to move
   * @param[in]  ref   what to use for the homing
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis is not specified.
   *
   * Homing can be performed using a reference switch (if equipped), or the 
   * positive or negative limit switches, as indicated by the "ref" argument.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_home_axis( const std::string &name, int addr, int axis, std::string ref ) {
    std::string function = "Physik_Instrumente::Interface::_home_axis";
    std::stringstream message;
    std::stringstream cmd;

    // if the axis is specified then make sure referencing method is selected
    //
    if ( addr > 0 ) {
      if ( addr > 0 ) cmd << addr << " ";
      cmd << "RON " << axis << " 1";
      if ( this->send_command( name, cmd.str() ) != NO_ERROR ) return ERROR;
    }

    // start building the reference command
    //
    cmd.str("");
    if ( addr > 0 ) cmd << addr << " ";

    // add the appropriate homing command based on the ref argument
    //
    if ( ref == "ref" ) cmd << "FRF";  // reference switch
    else
    if ( ref == "neg" ) cmd << "FNL";  // negative limit switch
    else
    if ( ref == "pos" ) cmd << "FPL";  // positive limit switch
    else {
      message.str(""); message << "ERROR: unknown homing reference " << ref << ". expected { ref | pos | neg }";
      logwrite( function, message.str() );
      return ERROR;
    }

    // then add the axis, if specified
    //
    if ( axis > 0 ) cmd << " " << axis;

    return( this->send_command( name, cmd.str() ) );
  }
  /***** Physik_Instrumente::Interface::_home_axis ****************************/


  /***** Physik_Instrumente::Interface::_home_axis_wait ***********************/
  /**
   * @brief      wait for the specified controller to be homed/referenced (private)
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller in daisy-chain
   * @param[in]  axis  axis to move
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_home_axis_wait( const std::string &name, int addr, int axis ) {
    std::string function = "Physik_Instrumente::Interface::_home_axis_wait";
    std::stringstream message;
    long error = NO_ERROR;

    this->motormap[name].axes[axis].ishome   = false;
    this->motormap[name].axes[axis].ontarget = false;

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    bool is_home=false;

    message.str(""); message << "homing " << name << " will time-out in " << this->home_timeout/1000. << " sec";
    logwrite( function, message.str() );

    do {
      bool state;
      this->is_home( name, addr, axis, state );
      this->motormap[name].axes[axis].ishome = state;
      is_home = this->motormap[name].axes[axis].ishome;

      if ( is_home ) break;
      else {
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > this->home_timeout ) {
        message.str(""); message << "ERROR home timeout waiting for " << name << " addr " << addr;
        logwrite( function, message.str() );
        error = TIMEOUT;
        break;
      }
    } while ( true );

    return error;
  }
  /***** Physik_Instrumente::Interface::_home_axis_wait ***********************/


  /***** Physik_Instrumente::Interface::_move_axis_wait ***********************/
  /**
   * @brief      wait for the specified controller to be moved
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller in daisy-chain
   * @param[in]  axis  axis to move
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_move_axis_wait( const std::string &name, int addr, int axis ) {
    std::string function = "Physik_Instrumente::Interface::_move_axis_wait";
    std::stringstream message;
    long error = NO_ERROR;

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    message.str(""); message << "moving " << name << " will time-out in " << this->move_timeout/1000. << " sec";
    logwrite( function, message.str() );

    do {
      bool state;
      this->on_target( name, addr, axis, state );
      this->motormap[name].axes[axis].ontarget = state;

      if ( this->motormap[name].axes[axis].ontarget ) break;
      else {
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > this->move_timeout ) {
        message.str(""); message << "ERROR move timeout waiting for " << name << " addr " << addr;
        logwrite( function, message.str() );
        error = TIMEOUT;
        break;
      }
    } while ( true );

    return error;
  }
  /***** Physik_Instrumente::Interface::_move_axis_wait ***********************/


  /***** Physik_Instrumente::Interface::is_home *******************************/
  /**
   * @brief      checks whether referencing has been done
   * @details    this is the outside-callable function
   * @param[in]  input      may contain list of motor names
   * @param[in]  retstring  reference to return string
   * @return     ERROR or NO_ERROR
   *
   * The input string can contain a space-delimited list of motor names,
   * or if empty then all motors will be used. Threads are not used here because
   * it's not a lengthy operation.
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::is_home( std::string input, std::string &retstring ) {
    std::string function = "Physik_Instrumente::Interface::is_home";
    std::stringstream message;
    std::vector<std::string> name_list;

    // If input is empty then build up a vector of each motor name
    //
    if ( input.empty() ) {
      for ( const auto &mot : this->motormap ) { name_list.push_back( mot.first ); }
    }
    else {
      Tokenize( input, name_list, " " );
    }

    // Loop through the name_list vector, asking each if homed,
    // keeping count of the number that are homed, which will be
    // compared against the number that were queried.
    //
    size_t num_home = 0;
    std::string homed, nothomed;
    long error = NO_ERROR;
    retstring.clear();

    for ( const auto &name : name_list ) {
      // requires an open connection
      //
      if ( this->is_connected( name ) ) {
        auto addr = this->motormap[name].addr;
        int axis = 1;
        bool _ishome;

        error |= this->is_home( name, addr, axis, _ishome );

        num_home += ( _ishome ? 1 : 0 );

        nothomed.append( _ishome ? "" : name ); nothomed.append( _ishome ? "" : " " );
        homed.append   ( _ishome ? name : "" ); homed.append   ( _ishome ? " " : "" );
      }
      else {
        message.str(""); message << "ERROR not connected to motor controller " << name;
        logwrite( function, message.str() );
        retstring="not_connected";
        error |= ERROR;
      }
    }

    // If all homed then the response is simply "true",
    // otherwise it is "false " followed by a list of names not homed.
    //
    if ( ! retstring.empty() ) { /* preserve any error */ }
    else
    if ( num_home == name_list.size() ) retstring = "true"; else retstring = "false ";

    // Log all, which are homed and which are not, if any
    //
    if ( ! homed.empty() ) {
      message.str(""); message << homed << "homed";
      logwrite( function, message.str() );
    }
    if ( ! nothomed.empty() ) {
      message.str(""); message << "NOTICE: " << nothomed << "not homed";
      logwrite( function, message.str() );
    }

    return error;
  }
  /***** Physik_Instrumente::Interface::is_home *******************************/


  /***** Physik_Instrumente::Interface::is_home *******************************/
  /**
   * @brief      queries whether referencing has been done and updates the class
   * @param[in]  name   controller name
   * @param[in]  addr   address of controller in daisy-chain
   * @param[in]  axis   axis to query
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::is_home( const std::string &name, int addr, int axis ) {
    std::string function = "Physik_Instrumente::Interface::is_home";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    bool state;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "FRF? " << axis;

    long error = this->send_command(name, cmd.str(), &reply);                  // send the command

    if ( error == NO_ERROR ) error = this->parse_reply( axis, reply, state );  // parse the response

    this->motormap[name].axes[axis].ishome   = state;

    return error;
  }
  /***** Physik_Instrumente::Interface::is_home *******************************/


  /***** Physik_Instrumente::Interface::is_home *******************************/
  /**
   * @brief      queries whether referencing has been done
   * @param[in]  name   controller name
   * @param[in]  addr   address of controller in daisy-chain
   * @param[in]  axis   axis to query
   * @param[out] state  reference to return state of home (true|false)
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::is_home( const std::string &name, int addr, int axis, bool &state ) {
    std::string function = "Physik_Instrumente::Interface::is_home";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "FRF? " << axis;

    long error = this->send_command(name, cmd.str(), &reply);                  // send the command

    if ( error == NO_ERROR ) error = this->parse_reply( axis, reply, state );  // parse the response

    return error;
  }
  /***** Physik_Instrumente::Interface::is_home *******************************/


  /***** Physik_Instrumente::Interface::on_target *****************************/
  /**
   * @brief      query the on target state for given addr and axis
   * @param[in]  name   controller name
   * @param[in]  addr   address of controller in daisy-chain
   * @param[in]  state  (true|false) if on target or not
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  template <typename ControllerType>
  long Interface<ControllerType>::on_target( const std::string &name, int addr, bool &state ) {
    return( this->on_target( name, addr, -1, state ) );  //!< all axes at this addr
  }
  /***** Physik_Instrumente::Interface::on_target *****************************/


  /***** Physik_Instrumente::Interface::on_target *****************************/
  /**
   * @brief      query the on target state for given addr and axis
   * @param[in]  name   controller name
   * @param[in]  addr   address of controller in daisy-chain
   * @param[in]  axis   axis to move
   * @param[in]  state  (true|false) if on target or not
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis is not specified.
   * 
   */
  template <typename ControllerType>
  long Interface<ControllerType>::on_target( const std::string &name, int addr, int axis, bool &state ) {
    std::string function = "Physik_Instrumente::Interface::on_target";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "ONT?";
    if ( axis > 0 ) cmd << " " << axis;

    long error = this->send_command(name, cmd.str(), &reply);                  // send the command

    if ( error == NO_ERROR ) error = this->parse_reply( axis, reply, state );  // parse the response

    return error;
  }
  /***** Physik_Instrumente::Interface::on_target *****************************/


  /***** Physik_Instrumente::Interface::get_pos *******************************/
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
                                          std::string* posname, float tol, int addr) {
    const std::string function("Physik_Instrumente::Interface::get_pos");

    // Check that requested name is defined in the motormap
    //
    auto name_found = this->motormap.find( name );
    if ( name_found == this->motormap.end() ) {
      logwrite(function, "ERROR name '"+name+"' not in configuration");
      if (posname) *posname="unknown_motor";
      return ERROR;
    }

    // Get the addr if not supplied
    //
    if (addr<0) addr = this->motormap[name].addr;

    // Check the axisnum
    //
    auto axis_found = this->motormap[name].axes.find( axisnum );
    if ( axis_found == this->motormap[name].axes.end() ) {
      logwrite(function, "ERROR axis '"+std::to_string(axisnum)+"' not in configuration for name '"+name+"'");
      if (posname) *posname="unknown_axis";
      return ERROR;
    }

    // Is this controller connected?
    //
    if ( ! this->is_connected( name ) ) {
      logwrite(function, "ERROR not connected to motor controller for '"+name+"'");
      if (posname) *posname="not_connected";
      return ERROR;
    }

    // motorname and axisnum are good and connected, so read the position
    //
    position=NAN;
    long error = this->_get_pos( name, axisnum, addr, position );

    // if tolerance not supplied then use the value from the class constructor
    //
    if (std::isnan(tol)) tol=this->tolerance;

    // does this position have a corresponding name in the posmap for this actuator?
    //
    for ( const auto &pos : this->motormap[name].posmap ) {
      if ( std::abs( pos.second.position - position ) < tol ) {
        if (posname) *posname = pos.second.posname;
        break;
      }
    }

    return error;
  }
  /***** Physik_Instrumente::Interface::get_pos *******************************/


  /***** Physik_Instrumente::Interface::_get_pos ******************************/
  /**
   * @brief      get the current position of a motor (private)
   * @param[in]  name  controller name
   * @param[in]  addr  address of controller in daisy-chain
   * @param[out] pos   position read
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_get_pos( const std::string &name, int addr, float &pos ) {
    return( this->_get_pos( name, addr, -1, pos ) );  //!< all axes at this addr
  }
  /***** Physik_Instrumente::Interface::_get_pos ******************************/


  /***** Physik_Instrumente::Interface::_get_pos ******************************/
  /**
   * @brief      get the current position of a motor (private)
   * @param[in]  name  controller name
   * @param[in]  axis  axis to read
   * @param[in]  addr  address of controller in daisy-chain
   * @param[out] pos   reference to position value read
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis is not specified.
   * 
   */
  template <typename ControllerType>
  long Interface<ControllerType>::_get_pos( const std::string &name, int axis, int addr, float &pos ) {
    std::string function = "Physik_Instrumente::Interface::_get_pos";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "POS?";
    if ( axis > 0 ) cmd << " " << axis;

    long error = this->send_command(name, cmd.str(), &reply);                  // send the command

    if ( error == NO_ERROR ) error = this->parse_reply( axis, reply, pos );    // parse the response

    return error;
  }
  /***** Physik_Instrumente::Interface::_get_pos ******************************/


  /***** Physik_Instrumente::Interface::stop_motion ***************************/
  /**
   * @brief      stop all movement on all axes
   * @param[in]  name  controller name
   * @param[in]  addr
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::stop_motion( const std::string &name, int addr ) {
    std::stringstream cmd;
    if ( addr > 0 ) cmd << addr << " ";
    cmd << "STP";
    this->send_command( name, cmd.str() );
    return NO_ERROR;
  }
  /***** Physik_Instrumente::Interface::stop_motion ***************************/


  /***** Physik_Instrumente::Interface::parse_reply ***************************/
  /**
   * @brief      parse the response from sending a ? command to the controller
   * @param[in]  axis number
   * @param[in]  reference to complete reply string
   * @return     code from controller, or -1 for internal error
   *
   * When sending a query command to the controller (one that ends in "?") then
   * a reply is read back; this function parses that reply.
   *
   * The reply from the controller is of the form:
   * 0 a A=R
   * where a is the addr, A is the axis number and R is the response.
   *
   */
  template <typename ControllerType>
  template <typename ReplyType>
  long Interface<ControllerType>::parse_reply( int axis, std::string &reply, ReplyType &retval ) {
    std::string function = "Physik_Instrumente::Interface::parse_reply";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::stringstream sep;

    if ( reply.empty() ) {
      logwrite( function, "ERROR: empty message" );
      return ERROR;
    }

    Tokenize( reply, tokens, "=" );

    // There must be two tokens. If not then the "=" is missing
    // and this is an error.
    //
    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR bad reply \"" << reply << "\": expected 2 tokens";
      logwrite( function, message.str() );
      return ERROR;
    }

    // The second token, tokens[1], contains the return value, R
    // which comes in as a string. Convert to the appropriate type
    // depending on the template type.
    //
    try {
      if constexpr( std::is_same_v<ReplyType, int> ) {    // convert to <int>
        retval = std::stoi( tokens.at(1) );
      }
      else
      if constexpr( std::is_same_v<ReplyType, float> ) {  // convert to <float>
        retval = std::stof( tokens.at(1) );
      }
      else
      if constexpr( std::is_same_v<ReplyType, bool> ) {   // convert to <bool>
        int tf = std::stoi( tokens.at(1) );
        if ( tf == 1 ) retval = true;
        else
        if ( tf == 0 ) retval = false;
        else {
          retval = false;
          message.str(""); message << "ERROR bad boolean " << tokens.at(1) << ": expected 1 or 0";
          logwrite( function, message.str() );
          return ERROR;
        }
      }
      else {
        logwrite( function, "ERROR unrecognized type: expected <int> or <float>" );
        return ERROR;
      }
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR bad reply \"" << reply << "\": unable to convert to integer: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch ( std::out_of_range & ) {
      logwrite( function, "ERROR reply or token out of range" );
      return ERROR;
    }

    return NO_ERROR;

  }
  // Explicit instantiation for possible types for this template function
  //
  template class Interface<StepperInfo>;
  template long Interface<StepperInfo>::parse_reply<int>( int axis, std::string &reply, int &retval );
  template long Interface<StepperInfo>::parse_reply<float>( int axis, std::string &reply, float &retval );
  template long Interface<StepperInfo>::parse_reply<bool>( int axis, std::string &reply, bool &retval );

  template class Interface<ServoInfo>;
  template long Interface<ServoInfo>::parse_reply<int>( int axis, std::string &reply, int &retval );
  template long Interface<ServoInfo>::parse_reply<float>( int axis, std::string &reply, float &retval );
  template long Interface<ServoInfo>::parse_reply<bool>( int axis, std::string &reply, bool &retval );

  template class Interface<PiezoInfo>;
  template long Interface<PiezoInfo>::parse_reply<int>( int axis, std::string &reply, int &retval );
  template long Interface<PiezoInfo>::parse_reply<float>( int axis, std::string &reply, float &retval );
  template long Interface<PiezoInfo>::parse_reply<bool>( int axis, std::string &reply, bool &retval );
  /***** Physik_Instrumente::Interface::parse_reply ***************************/


  /***** Physik_Instrumente::PiezoInfo::load_controller_info ******************/
  /**
   * @brief      Loads controller information from the config file into the class
   * @details    This is the derived class version which will parse the PiezoInfo
   *             specific arguments not handled by the template.
   * @param[in]  tokens  vector passed by ControllerInfo::load_controller_info()
   * @return     ERROR or NO_ERROR
   *
   * Not currently used
   *
   */
  long PiezoInfo::load_controller_info( std::vector<std::string> tokens ) {
    std::string function = "Physik_Instrumente::PiezoInfo::load_controller_info";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Physik_Instrumente::PiezoInfo::load_controller_info ******************/


  /***** Physik_Instrumente::ServoInfo::load_controller_info ******************/
  /**
   * @brief      Loads controller information from the config file into the class
   * @details    This is the derived class version which will parse the ServoInfo
   *             specific arguments not handled by the template.
   * @param[in]  tokens  vector passed by ControllerInfo::load_controller_info()
   * @return     NO_ERROR
   *
   * Not currently used
   *
   */
  long ServoInfo::load_controller_info( std::vector<std::string> tokens ) {
    std::string function = "Physik_Instrumente::ServoInfo::load_controller_info";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Physik_Instrumente::ServoInfo::load_controller_info ******************/


  /***** Physik_Instrumente::StepperInfo::load_controller_info ****************/
  /**
   * @brief      Loads controller information from the config file into the class
   * @details    This is the derived class version which will parse the StepperInfo
   *             specific arguments not handled by the template.
   * @param[in]  tokens  vector passed by ControllerInfo::load_controller_info()
   * @return     ERROR or NO_ERROR
   *
   * Not currently used
   *
   */
  long StepperInfo::load_controller_info( std::vector<std::string> tokens ) {
    std::string function = "Physik_Instrumente::StepperInfo::load_controller_info";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Physik_Instrumente::StepperInfo::load_controller_info ****************/

}
