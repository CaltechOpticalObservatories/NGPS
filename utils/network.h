/**
 * @file    network.h
 * @brief   network functions, specifically TCP/IP sockets
 * @details TcpSocket class definitions for client and server communications
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This contains the class member definitions for a TcpSocket class,
 * suitable for both client and server network communications, and
 * for the UdpSocket class, suitable for multicasters and subscribers.
 *
 */

#ifndef NEWTCPLINUX_H
#define NEWTCPLINUX_H

#include <chrono>                      // for timing timeouts
#include <cstdio>
#include <string>
#include <cstring>
#include <iostream>
#include <mutex>

#include <sys/ioctl.h>                 // for ioctl, FIONREAD
#include <poll.h>                      // for pollfd
#include <unistd.h>
#include <fcntl.h>

// for addrinfo
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// for inet_addr
#include <netinet/in.h>
#include <arpa/inet.h>

#include "logentry.h"                  // for logwrite() within the Network namespace

#define POLLTIMEOUT 6000               ///< default Poll timeout in msec
#define LISTENQ 64                     ///< listen(3n) backlog 
#define UDPMSGLEN 256                  ///< UDP message length

/***** Network ****************************************************************/
/**
 * @namespace Network
 * @brief     the network namespace contains classes for TCP and UDP socket communications
 *
 */
namespace Network {

  extern const long ERROR;
  extern const long NO_ERROR;
  extern const long TOUT;

  /***** TcpSocket ************************************************************/
  /**
   * @class  TcpSocket
   * @brief  TCP socket class
   *
   * This class contains everything needed for creating a TCP/IP socket,
   * for clients and servers, reading and writing, etc.
   *
   */
  class TcpSocket {

    private:
      uint16_t port;
      bool blocking;
      bool asyncflag;
      int totime;                      ///< timeout time for poll
      int fd;                          ///< connected socket file descriptor
      int listenfd;                    ///< listening socket file descriptor
      std::string host;
      bool connection_open;

      struct sockaddr_in cliaddr;        ///< socket address structure used for Accept
      socklen_t clilen;                  ///< size of the socket address structure

    public:
      TcpSocket();                       ///< basic class constructor
      TcpSocket(uint16_t port_in, bool block_in, int totime_in, int id_in);  ///< useful constructor for a server
      TcpSocket(uint16_t port_in, bool block_in, bool async_in, int totime_in, int id_in);  ///< useful constructor for a server
      TcpSocket( std::string host, uint16_t port_in, bool block_in, bool async_in, int totime_in, int id_in);  ///< useful constructor for a server
      TcpSocket( std::string host, uint16_t port );                          ///< client constructor
      TcpSocket(const TcpSocket &obj);   ///< copy constructor

      struct addrinfo *addrs;            ///< dynamically allocated linked list returned by getaddrinfo()

      int id;                            ///< id may be useful for tracking multiple threads, no real requirement here

      void set_totime( int time_in ) { this->totime = time_in; };
      int getfd() { return this->fd; };
      bool isblocking() { return this->blocking; };
      bool isasync() { return this->asyncflag; };
      bool isconnected() { return this->connection_open; };
      int polltimeout() { return this->totime; };
      void sethost(std::string host_in) { this->host = host_in; };
      std::string gethost() { return this->host; };
      void setport(uint16_t port_in) { this->port = port_in; };
      int getport() { return this->port; };

      int Accept();                      ///< creates a new connected socket for pending connection
      int Listen();                      ///< create a TCP listening socket
      int Poll();                        ///< polls a single file descriptor to wait for incoming data to read
      int Poll( int timeout );           ///< polls a single file descriptor with specified timeout
      int Connect();                     ///< connect to this->host on this->port
      int Close();                       ///< close a socket connection
      ssize_t Read( void* buf, const size_t count ); ///< read data from connected socket
      ssize_t Read(std::string &retstring);  ///< read data from connected socket until newline
      ssize_t Read( std::string &retstring, const char &term); ///< read data from connected socket until terminating char found
      ssize_t Read( std::string &retstring, const std::string &endstr);  ///< read data from connected socket until endstr
      int Bytes_ready();                 ///< get the number of bytes available on the socket descriptor this->fd
      void Flush();                      ///< flush a socket by reading until it's empty

      ssize_t Write(std::string msg_in); ///< write data to a socket

      ssize_t Write( const char* c );

      template <class T>
      ssize_t Write(T* buf, size_t count) {  ///< write raw data to a socket
        size_t bytes_sent=0;
        int    this_write=0;

        while ( bytes_sent < count ) {
          do {
            this_write = write( this->fd, buf, (count-bytes_sent) );
          } while ( (this_write < 0) && (errno == EINTR) );
          if (this_write <= 0) return this_write;
          bytes_sent += this_write;
          buf += this_write;
        }
        return bytes_sent;
      }
  };
  /***** TcpSocket ************************************************************/


  /***** UdpSocket ************************************************************/
  /**
   * @class  UdpSocket
   * @brief  UDP socket class
   *
   * This class contains everything needed for creating a UDP socket,
   * for listeners and broadcasters, reading and writing, etc.
   *
   */
  class UdpSocket {

    private:
      uint16_t port;                                        ///< port (for subscriber or broadcast)
      std::string group;                                    ///< group for multicast subscribers
      int fd;                                               ///< connected socket file descriptor
      struct sockaddr_in addr;                              ///< for UDP multicast
      struct ip_mreq mreq;                                  ///< multicast group addr stuct for UDP listener
      bool service_running;                                 ///< indicates UDP socket created and running

    public:
      UdpSocket();                                          ///< basic class constructor
      UdpSocket(uint16_t port_in, std::string group_in);    ///< useful constructor for a server
      UdpSocket(const UdpSocket &obj);                      ///< copy constructor
      ~UdpSocket();                                         ///< class destructor

      void setport(uint16_t port_in) { this->port = port_in; };  ///< use to set port when default constructor used
      int getport() { return this->port; };                 ///< use to get port
      bool is_running() { return this->service_running; };  ///< is the UDP service running?

      void setgroup(std::string group_in) { this->group = group_in; };   ///< use to set group when default constructor used
      std::string getgroup() { return this->group; };                    ///< use to get group

      int Create();                                         ///< create a UDP multi-cast socket
      int Send(std::string message);                        ///< transmit the message to the UDP socket
      int Close();                                          ///< close the UDP socket connection
      int Listener();                                       ///< creates a UDP listener, returns a file descriptor
      ssize_t Receive( std::string &message );              ///< receive a UDP message from the Listener fd
  };
  /***** UdpSocket ************************************************************/


  /***** Interface ************************************************************/
  /**
   * @class  Interface
   * @brief  interface class is generic interfacing to something via sockets
   *
   */
  class Interface {
    private:
      std::string name;           ///< a friendly name for info purposes
      std::string host;           ///< host name for the device
      uint16_t port;              ///< port number for device on host
      bool initialized;           ///< has the class been initialized?
      std::mutex mtx;
      char term_write;            ///< send_command() adds this char on writes
      char term_read;             ///< send_command() looks for this char on reads (if reply requested)

    public:
      /// has the class been initialized?
      bool is_initialized() { return this->initialized; };
      /// what is the name of the device?
      std::string get_name() { return this->name; };
      std::string get_host() { return this->host; };
      int get_port() { return this->port; };

      long open();                ///< open a connection to LKS device
      long close();               ///< close the connection to the LKS device
      long reconnect();
      long send_command( std::string cmd );
      long send_command( std::string cmd, std::string &retstring );
      long send_command( std::string cmd, std::string &retstring, int timeout );

      inline bool isopen() { std::lock_guard<std::mutex> lock( this->mtx ); return this->sock.isconnected(); }

      Interface( std::string name, std::string host, uint16_t port, char term_write, char term_read );
      Interface( std::string name, std::string host, uint16_t port );
      Interface();
      ~Interface();

      TcpSocket sock;    ///< provides the network communication

  };
  /***** Interface ************************************************************/

}
/***** Network ****************************************************************/
#endif
