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

/***** BrainBox ***************************************************************/
/**
 * @namespace BrainBox
 * @brief     namespace for the BrainBox interface
 *
 */
namespace BrainBox {

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

      enum class PinDirection {
	Input,
	Output
      };

      struct PinConfig {
	int          number;
	PinDirection direction;
	std::string  name;
      };

      Network::TcpSocket sock;    ///< provides the network communication

      bool get_initialized() { return this->is_initialized; };
      std::string get_name() { return this->name; };
      std::string get_model() { return this->model; };
      std::string get_location() { return this->location; };

      long configure_dio(const std::vector<std::string> &input);
      long open();
      long close();
      long send_command(std::string cmd);
      long send_command(std::string cmd, std::string &retstring);
      void read_digio();
      void write_digio(const uint8_t &byte);

      std::optional<int> validate_pin(const std::string &name) const;
      std::optional<PinConfig> validate_pinconfig(const int pin) const;
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
      std::mutex mtx;
      uint8_t diginstate;
      uint8_t digoutstate;
      std::unordered_map<int, PinConfig> pinmap;
      std::unordered_map<std::string, int> namemap;
  };
  /***** BrainBox::Interface **************************************************/


}
/***** BrainBox ***************************************************************/
