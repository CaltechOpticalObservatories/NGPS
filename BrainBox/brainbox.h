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
    private:
      std::string location;       ///< device location
      std::string model;          ///< model
      std::string name;           ///< a name for info purposes
      std::string host;           ///< host name for the device
      int port;                   ///< port number for device on host
      int address;                ///< device address
      bool is_initialized;        ///< has the class been initialized?
      std::mutex mtx;
      uint8_t raw{0};

    public:
      Interface(const std::string &host, const int port, const int address);
      Interface();
      ~Interface();

      Network::TcpSocket sock;    ///< provides the network communication

      bool get_initialized() { return this->is_initialized; };
      std::string get_name() { return this->name; };
      std::string get_model() { return this->model; };
      std::string get_location() { return this->location; };

      long open();
      long close();
      long send_command(std::string cmd);
      long send_command(std::string cmd, std::string &retstring);
      long read_digio();
      long write_digio();
      bool is_bit_set(uint8_t bit);
      bool is_bit_set(uint8_t raw, uint8_t bit);
  };
  /***** BrainBox::Interface **************************************************/


}
/***** BrainBox ***************************************************************/
