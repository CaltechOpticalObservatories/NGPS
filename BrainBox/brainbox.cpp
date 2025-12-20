/**
 * @file    brainbox.cpp
 * @brief   this file contains the code for interfacing with BrainBox hardware
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "brainbox.h"
#include "logentry.h"

namespace BrainBox {

  /***** BrainBox::Interface::Interface ***************************************/
  /**
   * @brief      default Interface class constructor
   *
   */
  Interface::Interface()
    : port(-1),
      address(-1),
      is_initialized(false)
  {
  }
  /***** BrainBox::Interface::Interface ***************************************/


  /***** BrainBox::Interface::Interface ***************************************/
  /**
   * @brief      Interface class constructor
   * @param[in]  host     hostname of the device
   * @param[in]  port     port number of the device
   * @param[in]  address  address of the device
   *
   */
  Interface::Interface(const std::string &host, const int port, const int address)
    : host(host),
      port(port),
      address(address),
      is_initialized(true),
      diginstate(0),
      digoutstate(0)
  {
  }
  /***** BrainBox::Interface::Interface ***************************************/


  /***** BrainBox::Interface::~Interface **************************************/
  /**
   * @brief      Interface class deconstructor
   *
   */
  Interface::~Interface() {
  };
  /***** BrainBox::Interface::~Interface **************************************/


  /***** BrainBox::Interface::configure_dio ****************************************/
  /**
   * @brief      parse and apply the BLUE_LAMP_DIO lines from the config file
   * @param[in]  input  vector containing the BLUE_LAMP_DIO config lines
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::configure_dio(const std::vector<std::string> &input) {
    const std::string function("BrainBox::Interface::configure_dio");
    std::ostringstream message;

    this->pinmap.clear();
    this->namemap.clear();

    // each vector element is a line from the config file
    for (const auto &line : input) {
      // tokenize the input config line
      std::vector<std::string> tokens;
      Tokenize(line, tokens, " ");

      if (tokens.size() != 3) {
        logwrite(function, "ERROR exepected <pin#> <direction> <name>");
        return ERROR;
      }

      try {
        int pin           = std::stoi(tokens.at(0));
        std::string dirin = to_uppercase(tokens.at(1));
        std::string name  = tokens.at(2);

	if (pin < 0 || pin > 7) {
          logwrite(function, "ERROR invalid pin '"+tokens.at(0)+"' must be in range {0:7}");
          return ERROR;
        }

	if (dirin != "INPUT" && dirin != "OUTPUT") {
          logwrite(function, "ERROR invalid direction '"+tokens.at(1)+"' must be input|output");
          return ERROR;
        }
	auto dir = (dirin=="INPUT") ? PinDirection::Input : PinDirection::Output;

	// insert this row into the maps
	this->pinmap[pin]   = {pin, dir, name};
	this->namemap[name] = pin;
      }
      catch (const std::exception &e) {
        logwrite(function, "ERROR parsing <pin#> <direction> <name> : "+std::string(e.what()));
        return ERROR;
      }
    }

    return NO_ERROR;
  }
  /***** BrainBox::Interface::configure_dio ****************************************/


  /***** BrainBox::Interface::open *************************************************/
  /**
   * @brief      open a connection to the BrainBox hardware
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    const std::string function("BrainBox::Interface::open");
    std::ostringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( this->sock.isconnected() ) {
      message << "socket connection already open to "
              << this->sock.gethost() << ":" << this->sock.getport()
              << " on fd " << this->sock.getfd() << " for BrainBox " << this->model
              << " (" << this->name << ")";
      logwrite(function, message.str());
      return NO_ERROR;
    }

    Network::TcpSocket s( this->host, this->port );
    this->sock = s;

    // Initialize connection to the BrainBox
    //
    if ( this->sock.Connect() != 0 ) {
      logwrite(function, "ERROR opening connection to "+this->sock.gethost());
      return ERROR;
    }

    message << "socket connection to " 
            << this->sock.gethost() << ":" << this->sock.getport()
            << " established on fd " << this->sock.getfd();
    logwrite(function, message.str());

    return NO_ERROR;
  }
  /***** BrainBox::Interface::open ********************************************/


  /***** BrainBox::Interface::close *******************************************/
  /**
   * @brief      close the connection to the BrainBox socket
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    const std::string function("BrainBox::Interface::close");
    std::ostringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      logwrite(function, "socket connection not open to "+this->sock.gethost());
      return NO_ERROR;
    }

    long error = this->sock.Close();

    if (error == NO_ERROR) {
      logwrite(function, "socket connection to "+this->sock.gethost()+" closed");
    }
    else {
      logwrite(function, "ERROR closing socket connection to "+this->sock.gethost());
    }

    return error;
  }
  /***** BrainBox::Interface::close *******************************************/


  /***** BrainBox::Interface::send_command ************************************/
  /**
   * @brief      send specified command to BrainBox socket interface
   * @details    This function is overloaded. This version discards the reply.
   * @param[in]  cmd        command to send
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string dontcare;
    return this->send_command( cmd, dontcare );
  }
  /***** BrainBox::Interface::send_command ************************************/


  /***** BrainBox::Interface::send_command ************************************/
  /**
   * @brief      send specified command to BrainBox socket interface and return reply
   * @details    This function is overloaded. This version returns the reply.
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply read back from device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring ) {
    const std::string function("BrainBox::Interface::send_command");
    std::ostringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      logwrite(function, "ERROR not connected to BrainBox");
      return ERROR;
    }

    // send the command
    cmd.append( "\r" );                            // add the CR character
    int written = this->sock.Write( cmd );         // write the command
    if ( written <= 0 ) {                          // return error if error writing to socket
      logwrite(function, "ERROR sending \"" + strip_newline(cmd) + "\" to BrainBox");
      return ERROR;
    }

    // read the reply
    while ( error == NO_ERROR && retval >= 0 ) {

      if ( ( retval=this->sock.Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "TIMEOUT on fd " << this->sock.getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "ERROR on fd " << this->sock.getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = this->sock.Read( reply, '\r' ) ) < 0 ) {
        message.str(""); message << "ERROR: " << strerror( errno ) 
                                 << ": reading from " << this->sock.gethost() << "/" << this->sock.getport();
        logwrite(function, message.str());
        break;
      }

      // remove any newline characters and get out
      reply = strip_newline(reply);
      break;
    }
    retstring = reply;

    return error;
  }
  /***** BrainBox::Interface::send_command ************************************/


  /***** BrainBox::Interface::get_dionames ************************************/
  /**
   * @brief      returns a vector of names of all configured dio pins
   * @return     vector<string>
   */
  std::vector<std::string> Interface::get_dionames() const {
    std::vector<std::string> names;
    for (const auto &[name,pin] : this->namemap) {
      names.push_back(name);
    }
    return names;
  }
  /***** BrainBox::Interface::get_dionames ************************************/


  /***** BrainBox::Interface::validate_pin ************************************/
  /**
   * @brief      returns a pin for specified name if valid
   * @param[in]  name  name of pin
   * @return     pin number or nullopt
   */
  std::optional<int> Interface::validate_pin(const std::string &name) const {
    auto it = this->namemap.find(name);
    if (it == this->namemap.end()) return std::nullopt;
    return it->second;
  }
  /***** BrainBox::Interface::validate_pin ************************************/


  /***** BrainBox::Interface::validate_pinconfig ******************************/
  /**
   * @brief      returns a pin config struct if the pin is valid
   * @param[in]  pin  pin number
   * @return     PinConfig or nullopt
   */
  std::optional<Interface::PinConfig> Interface::validate_pinconfig(const int pin) const {
    auto pinmap_it = this->pinmap.find(pin);
    if (pinmap_it == this->pinmap.end()) return std::nullopt;
    return pinmap_it->second;
  }
  /***** BrainBox::Interface::validate_pinconfig ******************************/


  /***** BrainBox::Interface::set_bit *****************************************/
  void Interface::set_bit(const std::string &name, bool state) {
    const std::string function("BrainBox::Interface::set_bit");

    auto goodpin = this->validate_pin(name);
    if (!goodpin) throw std::runtime_error(function+": invalid name '"+name+"'");

    int pin = *goodpin;

    auto goodpinconfig = this->validate_pinconfig(pin);
    if (!goodpinconfig) throw std::runtime_error(function+": invalid pin");

    const auto &config = *goodpinconfig;

    // Pin direction must be configured for Output
    if (config.direction != PinDirection::Output) {
      throw std::runtime_error(function+" pin '"+name+"' is not an output pin");
    }

    if (state) {
      // set bit
      this->digoutstate |= (uint8_t)(1u << pin);
    }
    else {
      // clear bit
      this->digoutstate &= ~(uint8_t)(1u << pin);
    }

    try { this->write_digio(this->digoutstate); } catch (...) {throw;}
  }
  /***** BrainBox::Interface::set_bit *****************************************/


  /***** BrainBox::Interface::get_diostate ************************************/
  /**
   * @brief      get the state of a pin
   * @details    Uses optional so a true|false state is returned only if the
   *             pin is valid and configured. This function is overloaded.
   * @param[in]  name  name of pin to check
   * @return     true|false|null
   *
   */
  std::optional<bool> Interface::get_diostate(const std::string &name) {
    auto goodpin = this->validate_pin(name);
    if (!goodpin) return std::nullopt;

    int pin = *goodpin;

    auto goodpinconfig = this->validate_pinconfig(pin);
    if (!goodpinconfig) return std::nullopt;

    const auto &config = *goodpinconfig;

    try {
      this->read_digio();
    }
    catch (const std::exception &e) {
      logwrite("BrainBox::Interface::get_diostate", "ERROR "+std::string(e.what()));
      return std::nullopt;
    }

    // mask the desired pin in the state byte
    uint8_t mask = (uint8_t)(1u << pin);

    if (config.direction == PinDirection::Input) {
      return (this->diginstate & mask) != 0;
    }
    else {
      return (this->digoutstate & mask) != 0;
    }
  }
  /***** BrainBox::Interface::get_diostate ************************************/


  /***** BrainBox::Interface::get_diostate ************************************/
  /**
   * @brief      get the state of a pin
   * @details    Uses optional so a true|false state is returned only if the
   *             pin is valid and configured. This function is overloaded.
   * @param[in]  pin  pin number to check
   * @return     true|false|null
   */
  std::optional<bool> Interface::get_diostate(int pin) {
    if (pin<0 || pin>7) return std::nullopt;
    auto it = this->pinmap.find(pin);
    if (it == this->pinmap.end()) return std::nullopt;

    try {
      this->read_digio();
    }
    catch (const std::exception &e) {
      logwrite("BrainBox::Interface::get_diostate", "ERROR "+std::string(e.what()));
      return std::nullopt;
    }

    uint8_t mask = 1 << pin;

    if (it->second.direction == PinDirection::Input) {
      return (this->diginstate & mask) != 0;
    }
    else {
      return (this->digoutstate & mask) != 0;
    }
  }
  /***** BrainBox::Interface::get_diostate ************************************/


  /***** BrainBox::Interface::read_digio **************************************/
  /**
   * @brief      read digital IO lines
   * @throws     std::runtime_error
   *
   */
  void Interface::read_digio() {
    const std::string function("BrainBox::Interface::read_digio");
    std::string retstring;
    std::ostringstream cmd;

    cmd << "@" << std::setw(2) << std::setfill('0') << static_cast<unsigned>(this->address);

    long error = this->send_command(cmd.str(), retstring);

    if (error != NO_ERROR) {
      throw std::runtime_error(function+": failed to send command");
    }

    if (retstring.size() == 0) throw std::runtime_error(function+": empty response");
    if (retstring.size() != 5) throw std::runtime_error(function+": invalid response '"+retstring+"'");

    if (retstring[0] != '>') throw std::runtime_error(function+": invalid command");

    retstring.erase(0,1);

    logwrite(function, retstring);

    try {
      uint16_t state = static_cast<uint16_t>(std::stoul(retstring, nullptr, 16));
      this->digoutstate = static_cast<uint8_t>( (state >> 8) & 0xFF );
      this->diginstate  = static_cast<uint8_t>( state & 0xFF );
    }
    catch(const std::exception &e) {
      throw std::runtime_error(function+": invalid response '"+retstring+"'");
    }
  }
  /***** BrainBox::Interface::read_digio **************************************/


  /***** BrainBox::Interface::write_digio *************************************/
  /**
   * @brief      write digital IO lines
   * @throws     std::runtime_error
   *
   */
  void Interface::write_digio(const uint8_t &byte) {
    const std::string function("BrainBox::Interface::write_digio");
    std::string retstring;
    std::ostringstream cmd;

    cmd << "@"
        << std::hex << std::setfill('0')
	<< std::setw(2) << static_cast<unsigned>(this->address)
	<< std::setw(2) << static_cast<unsigned>(byte);

    long error = this->send_command(cmd.str(), retstring);

    if (error != NO_ERROR) {
      throw std::runtime_error(function+": failed to send command");
    }

    if (retstring.size() == 0) throw std::runtime_error(function+": empty response");
    if (retstring[0] != '>') throw std::runtime_error(function+": invalid command");

    try { this->read_digio(); } catch (...) { throw; }
  }
  /***** BrainBox::Interface::write_digio *************************************/

}
