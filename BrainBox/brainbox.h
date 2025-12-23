/**
 * @file    brainbox.h
 * @brief   this file contains the definitions for the BrainBox DIO interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "common.h"
#include "network.h"
#include "utilities.h"

#include <string>
#include <vector>
#include <regex>
#include <mutex>
#include <optional>

/***** BrainBox ***************************************************************/
/**
 * @namespace BrainBox
 * @brief     namespace for the BrainBox interface
 *
 */
namespace BrainBox {

  class Interface;

  enum class PinDirection { Input, Output };

  static constexpr std::array<std::pair<std::string_view, PinDirection>, 2>
  direction_table{ {
    { "INPUT",  PinDirection::Input },
    { "OUTPUT", PinDirection::Output }
  }};

  struct ConfigLine {
    std::string name;
    int pin;
    PinDirection direction;
  };

  inline ConfigLine parse_config_line(std::string line) {
    ConfigLine config;
    std::istringstream iss;
    std::string dirname;

    if (!(iss >> config.name >> config.pin >> dirname)) {
      throw std::runtime_error("invalid config: '"+line+"'");
    }

    make_uppercase(dirname);

    auto direction = direction_from_string(dirname);
    if (!direction) throw std::runtime_error("invalid direction: '"+dirname+"'");
    config.direction = direction;

    return config;
  }


  /***** BrainBox::Interface **************************************************/
  /**
   * @class  Interface
   * @brief  interface class is generic interfacing to BrainBox DIO via sockets
   *
   */
  class Interface {
    public:
      Interface(const std::string &host, const int port, const int address);
      Interface();
      ~Interface();

      struct PinConfig {
	int          number;
	PinDirection direction;
      };

      Network::TcpSocket sock;    ///< provides the network communication

      bool get_initialized() { return this->is_initialized; };
      std::string get_name() { return this->name; };
      std::string get_model() { return this->model; };
      std::string get_location() { return this->location; };

      long open();
      long close();

      long configure_pins(const std::vector<PinConfig> &pins);
      void set_pin(int pin, bool state);
      std::optional<bool> get_pin(int pin) const;

      PinDirection direction_of(int pin) const;

      std::optional<std::string_view> string_from_direction(PinDirection dirin);
      std::optional<PinDirection> direction_from_string(std::string_view namein);

      long configure_dio(const std::vector<std::string> &input);
      long send_command(std::string cmd);
      long send_command(std::string cmd, std::string &retstring);
      void read_digio();
      void write_digio(const uint8_t &byte);

      const PinConfig* validate_pin(const int pin) const;
      void set_bit(const std::string &name, bool state);
      std::optional<bool> get_diostate(int pin);
      std::optional<bool> get_diostate(const std::string &name);
      std::vector<std::string> get_dionames() const;

    private:
      std::string location;       ///< device location
      std::string model;          ///< model
      std::string name;           ///< a name for info purposes
      std::string host;           ///< host name for the device
      int port;                   ///< port number for device on host
      int address;                ///< device address
      bool is_initialized;        ///< has the class been initialized?
      uint8_t diginstate;
      uint8_t digoutstate;
      std::unordered_map<int, PinConfig> pinmap;
      std::unordered_map<std::string, int> namemap;
      mutable std::mutex mtx;
  };
  /***** BrainBox::Interface **************************************************/


}
/***** BrainBox ***************************************************************/
