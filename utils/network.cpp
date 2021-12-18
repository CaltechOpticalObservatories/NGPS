/**
 * @file    network.cpp
 * @brief   network functions, specifically TCP/IP sockets
 * @details TcpSocket class for client and server communications
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This contains the class member functions for a TcpSocket class,
 * suitable for both client and server network communications.
 *
 * For a client:
 * set this->host, this->port and call Connect(). 
 *
 * For a server:
 * at minimum, set this->port, this->blocking or instantiate using
 * (port, blocking, totime, id). Call Listen() then Accept(). 
 *
 * For both:
 * Call Read() and Write() to read/write from/to the socket. 
 * Use Poll() to wait for data to be ready to read.
 *
 */

#include "network.h"
#include "logentry.h"  // for logwrite() within the Network namespace

namespace Network {


  /**************** Network::UdpSocket::UdpSocket *****************************/
  /**
   * @fn     UdpSocket
   * @brief  UdpSocket class constructor
   * @param  port_in, port on which server will multicast datagrams
   * @param  group_in, 
   * @return none
   *
   * Use this to construct a UDP multi-cast datagram server object
   *
   */
  UdpSocket::UdpSocket(int port_in, std::string group_in) {
    this->fd = -1;
    this->port = port_in;
    this->group = group_in;
    this->service_running = false;
  }
  /**************** Network::UdpSocket::UdpSocket *****************************/


  /**************** Network::UdpSocket::UdpSocket *****************************/
  /**
   * @fn     UdpSocket
   * @brief  default UdpSocket class constructor
   * @param  none
   * @return none
   *
   */
  UdpSocket::UdpSocket() {
    this->fd = -1;
    this->port = -1;
    this->group = "";
    this->service_running = false;
  }
  /**************** Network::UdpSocket::UdpSocket *****************************/


  /**************** Network::UdpSocket::UdpSocket *****************************/
  /**
   * @fn     UdpSocket
   * @brief  UdpSocket class copy constructor
   * @param  obj reference to class object
   * @return none
   *
   */
  UdpSocket::UdpSocket(const UdpSocket &obj) {
    fd = obj.fd;
    port = obj.port;
    group = obj.group;
    service_running = obj.service_running;
  }
  /**************** Network::UdpSocket::UdpSocket *****************************/


  /**************** Network::UdpSocket::~UdpSocket ****************************/
  /**
   * @fn     ~UdpSocket
   * @brief  UdpSocket class deconstructor
   * @param  none
   * @return none
   *
   */
  UdpSocket::~UdpSocket() {
    this->Close();
  };
  /**************** Network::UdpSocket::~UdpSocket ****************************/


  /**************** Network::UdpSocket::Create ********************************/
  /**
   * @fn     Create
   * @brief  create a UDP multi-cast socket
   * @param  none
   * @return 0 on success, -1 on error, 1 to indicate user-requested disable
   *
   * If the .cfg file contains ASYNCGROUP=none (any case) then do not create
   * the socket. Return a 1 so the caller knows that no socket was created
   * by user request, as opposed to an error.
   *
   */
  int UdpSocket::Create() {
    std::string function = "Network::UdpSocket::Create";
    std::stringstream errstm;

    // don't create more than one UDP multicast socket
    //
    if (this->service_running) {
      logwrite(function, "ERROR: service already running");
      return -1;
    }

    // don't do anything if the ASYNCGROUP is not initialized
    //
    if (this->group.empty()) {
      logwrite(function, "ERROR: ASYNCGROUP not initialized. Cannot create socket");
      return -1;
    }

    // the user can set ASYNCGROUP=none to disable the async status message port
    //
    try {
      std::transform( this->group.begin(), this->group.end(), this->group.begin(), ::toupper );    // make uppercase
    }
    catch (...) {
      logwrite(function, "error converting ASYNCGROUP to uppercase");
      return -1;
    }
    if (this->group == "NONE") {
      logwrite(function, "ASYNCGROUP=none. UDP multicast socket disabled.");
      return 1;
    }

    // now that there is a group, check that the port is initialized
    //
    if (this->port < 0) {
      logwrite(function, "ERROR: ASYNCPORT not initialized. Cannot create socket");
      return -1;
    }

    // now that there is a group and port, create the socket
    //
    if ( (this->fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 ) {
      errstm << "error " << errno << " creating socket: " << strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    // must enable loopback to allow more than one listener
    //
    int loopback = 1;
    setsockopt(this->fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loopback, sizeof(loopback));

    // set number of hops to 2
    //
    int timetolive = 2;
    setsockopt(this->fd, IPPROTO_IP, IP_MULTICAST_LOOP, &timetolive, sizeof(timetolive));

    // set up the source address
    //
    memset((struct sockaddr *) &this->addr, 0, (socklen_t)sizeof(this->addr));
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = inet_addr(this->group.c_str());
    this->addr.sin_port = htons(this->port);
    setsockopt (this->fd, IPPROTO_IP, IP_MULTICAST_IF, &this->addr, sizeof(this->addr));

    this->service_running = (this->fd >= 0 ? true : false);

    return 0;
  }
  /**************** Network::UdpSocket::Create ********************************/


  /**************** Network::UdpSocket::Send **********************************/
  /**
   * @fn     Send
   * @brief  transmit the message to the UDP socket
   * @param  none
   * @return 0 on success, -1 on error
   *
   */
  int UdpSocket::Send(std::string message) {
    std::string function = "Network::UdpSocket::Send";
    std::stringstream errstm;
    int nbytes;

    if ( !this->is_running() ) return 0;  // silently do nothing if the UDP multicast socket isn't running

    if ( (nbytes = sendto( this->fd, message.c_str(), (size_t)message.length(), 0, (struct sockaddr*) &this->addr, (socklen_t)sizeof(this->addr) )) < 0 ) {
      errstm << "error " << errno << " calling sendto: " << strerror(errno);
      logwrite(function, errstm.str());
      return -1;
    }

    return 0;
  }
  /**************** Network::UdpSocket::Send **********************************/


  /**************** Network::UdpSocket::Close *********************************/
  /**
   * @fn     Close
   * @brief  close the UDP socket connection
   * @param  none
   * @return 0 on success, -1 on error
   *
   */
  int UdpSocket::Close() {
    int error = -1;
    if (this->fd >= 0) {               // if the file descriptor is valid
      if (close(this->fd) == 0) {      // then close it
        error = 0;
        this->fd = -1;
      }
      else {
        error = -1;                    // error closing file descriptor
     }
    }
    else {
      error = 0;                       // didn't start with a valid file descriptor
    }

    this->service_running = false;     // clear the service_running flag
    return (error);
  }
  /**************** Network::UdpSocket::Close *********************************/


  /**************** Network::TcpSocket::TcpSocket *****************************/
  /**
   * @fn     TcpSocket
   * @brief  TcpSocket class constructor
   * @param  port_in, port which server will listen on
   * @param  block_in, true|false -- will the connection be blocking?
   * @param  totime_in, timeout time for poll, in msec
   * @param  id_in, ID number (for keeping track of threads)
   * @return none
   *
   * Use this to construct a server's listening socket object
   *
   */
  TcpSocket::TcpSocket(int port_in, bool block_in, int totime_in, int id_in) {
    this->port = port_in;
    this->blocking = block_in;
    this->totime = totime_in;
    this->id = id_in;
    this->fd = -1;
    this->listenfd = -1;
    this->host = "";
    this->addrs = NULL;
    this->connection_open = false;
  };
  /**************** Network::TcpSocket::TcpSocket *****************************/


  /**************** Network::TcpSocket::TcpSocket *****************************/
  /**
   * @fn     TcpSocket
   * @brief  TcpSocket class constructor
   * @param  host, hostname to which client will connect
   * @param  port_in, port to which client will connect
   * @return none
   *
   * Use this to construct a client object
   *
   */
  TcpSocket::TcpSocket( std::string host, int port ) {
    this->host = host;
    this->port = port;
    this->totime = POLLTIMEOUT;    //!< default Poll timeout in msec
    this->fd = -1;
    this->addrs = NULL;
    this->connection_open = false;
  }
  /**************** Network::TcpSocket::TcpSocket *****************************/


  /**************** Network::TcpSocket::TcpSocket *****************************/
  /**
   * @fn     TcpSocket
   * @brief  default TcpSocket class constructor
   * @param  none
   * @return none
   *
   */
  TcpSocket::TcpSocket() {
    this->port = -1;
    this->blocking = false;
    this->totime = POLLTIMEOUT;    //!< default Poll timeout in msec
    this->id = -1;
    this->fd = -1;
    this->listenfd = -1;
    this->host = "";
    this->addrs = NULL;
    this->connection_open = false;
  };
  /**************** Network::TcpSocket::TcpSocket *****************************/


  /**************** Network::TcpSocket::TcpSocket *****************************/
  /**
   * @fn     TcpSocket
   * @brief  TcpSocket class copy constructor
   * @param  obj reference to class object
   * @return none
   *
   */
  TcpSocket::TcpSocket(const TcpSocket &obj) {
    port = obj.port;
    blocking = obj.blocking;
    totime = obj.totime;
    id = obj.id;
    fd = obj.fd;
    listenfd = obj.listenfd;
    host = obj.host;
    addrs = obj.addrs;
    connection_open = obj.connection_open;
  };
  /**************** Network::TcpSocket::TcpSocket *****************************/


  /**************** Network::TcpSocket::~TcpSocket ****************************/
  /**
   * @fn     ~TcpSocket
   * @brief  TcpSocket class deconstructor
   * @param  none
   * @return none
   *
   */
  TcpSocket::~TcpSocket() {
    this->Close();
  };
  /**************** Network::TcpSocket::~TcpSocket ****************************/


  /**************** Network::TcpSocket::Accept ********************************/
  /**
   * @fn     Accept
   * @brief  creates a new connected socket for pending connection
   * @param  none
   * @return fd (new connected socket file descriptor) or -1 on error
   *
   * Create new listening socket for pending connection on this->listenfd
   * and returns a new connected socket this->fd
   *
   */
  int TcpSocket::Accept() {
    this->clilen = (socklen_t)sizeof(this->cliaddr);
    this->fd = accept(this->listenfd, (struct sockaddr *) &this->cliaddr, &this->clilen);
    if (this->fd < 0) { perror("(Network::TcpSocket::Accept) error calling accept"); return -1; }
    return (this->fd);
  }
  /**************** Network::TcpSocket::Accept ********************************/


  /**************** Network::TcpSocket::Listen ********************************/
  /**
   * @fn     Listen
   * @brief  create a TCP listening socket
   * @param  none
   * @return listenfd (listening file descriptor), or -1 on error
   *
   * This creates a TCP listening socket that will be used to accept
   * incomming connections.
   *
   */
  int  TcpSocket::Listen() {
    std::string function = "Network::TcpSocket::Listen";
    std::stringstream errstm;

    // create the socket
    //
    if ( (this->listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 ) {
      errstm << "error " << errno << " creating socket: " << strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    // allow re-binding to port while previous connection is in TIME_WAIT
    //
    int on=1;
    setsockopt(this->listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // allow kernel to linger on close --
    // if there is any data remaining in the socket send buffer then then process
    // sleeps until data is sent and acknowledged by the peer or until POLLTIMEOUT.
    //
    struct linger so_linger;
    so_linger.l_onoff = 1;
    so_linger.l_linger = POLLTIMEOUT;
    setsockopt( this->listenfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger) );

    // get address information and bind it to the socket
    //
    struct sockaddr_in servaddr = {0};
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(this->port);

    if ( bind(this->listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
      errstm << "error " << errno << " binding to fd " << this->listenfd << ": " << strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    // start listening
    //
    if (listen(this->listenfd, LISTENQ)!=0) {
      errstm << "error " << errno << " listening to fd " << this->listenfd << ": " << strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    return(this->listenfd);
  }
  /**************** Network::TcpSocket::Listen ********************************/


  /**************** Network::TcpSocket::Poll **********************************/
  /**
   * @fn     Poll
   * @brief  polls a single file descriptor to wait for incoming data to read
   * @param  none
   * @return 0 on timeout, -1 on error, a non-negative value on success
   *
   */
  int TcpSocket::Poll() {
    struct pollfd poll_struct;
    poll_struct.events = POLLIN;
    poll_struct.fd     = this->fd;
    return poll(&poll_struct, 1, this->totime);
  }
  /**************** Network::TcpSocket::Poll **********************************/


  /**************** Network::TcpSocket::Connect *******************************/
  /**
   * @fn     Connect
   * @brief  connect to this->host on this->port
   * @param  none
   * @return 0 on success, -1 on error
   *
   * this->host and this->port need to be specified prior to calling Connect()
   * On success, this function will set this->fd with the open socket file descriptor.
   *
   * getaddrinfo() will allocate memory which must be freed by TcpSocket::Close()
   *
   */
  int TcpSocket::Connect() {
    std::string function = "Network::TcpSocket::Connect";
    std::stringstream errstm;

    struct addrinfo hints = {0};       //!< destination for getaddrinfo

    hints.ai_family = AF_INET;         //!< IPv4 address only
    hints.ai_socktype = SOCK_STREAM ;  //!< non-blocking, connection-based socket
    hints.ai_protocol = IPPROTO_TCP;   //!< TCP socket
    hints.ai_flags = AI_NUMERICSERV;   //!< numeric port number

    // Return one (or more) socket address structures which identify the requested host and port
    // Call TcpSocket::Close() to free memory allocated by this call.
    //
    int status = getaddrinfo(this->host.c_str(), std::to_string(this->port).c_str(), &hints, &this->addrs);

    if ( status != 0 ) {
      errstm << "error " << errno << " connecting to " << this->host << "/" << this->port
                         << " : " << gai_strerror(status);
      logwrite(function, errstm.str());
      return(-1);
    }

    // Loop though the results returned by getaddrinfo and attempt to create a socket and connect to it.
    // There should only be one when a host:port is specified but this form is more general.
    //
    for (struct addrinfo *sa = this->addrs; sa != NULL; sa = sa->ai_next) {
      // create the socket, which returns a file descriptor on success
      //
      this->fd = socket(sa->ai_family, sa->ai_socktype, sa->ai_protocol);

      if (this->fd == -1) continue;    // try another entry in the sock addr struct

      // connect to the socket file descriptor
      //
      if ( connect( this->fd, sa->ai_addr, sa->ai_addrlen ) == -1 ) {
        errstm << "error " << errno << " connecting to " << this->host << "/" << this->port 
                           << " on fd " << this->fd << ": " << std::strerror(errno);
        logwrite(function, errstm.str());
        return (-1);
      } else break;
    }

    int flags;
    if ((flags = fcntl(this->fd, F_GETFL, 0)) < 0) {
      errstm << "error " << errno << " getting socket file descriptor flags: " << std::strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    flags |= O_NONBLOCK;

    if (fcntl(this->fd, F_SETFL, flags) < 0) {
      errstm << "error " << errno << " setting socket file descriptor flags: " << std::strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    this->connection_open = (this->fd >= 0 ? true : false);

    return (this->fd >= 0 ? 0 : -1);
  }
  /**************** Network::TcpSocket::Connect *******************************/


  /**************** Network::TcpSocket::Close *********************************/
  /**
   * @fn     Close
   * @brief  close a socket connection and free memory allocated by getaddrinfo()
   * @param  none
   * @return 0 on success, -1 on error
   *
   */
  int TcpSocket::Close() {
    int error = -1;
    if (this->fd >= 0) {               // if the file descriptor is valid
      if (close(this->fd) == 0) {      // then close it
        error = 0;
        this->fd = -1;
      }
      else {
        error = -1;                    // error closing file descriptor
     }
    }
    else {
      error = 0;                       // didn't start with a valid file descriptor
    }

    if (this->addrs != NULL) {         // free memory allocated by getaddrinfo()
      freeaddrinfo(this->addrs);
      this->addrs = NULL;
    }

    this->connection_open = false;     // clear the connection_open flag
    return (error);
  }
  /**************** Network::TcpSocket::Close *********************************/


  /**************** Network::TcpSocket::Write *********************************/
  /**
   * @fn     Write
   * @brief  write data to a socket
   * @param  msg_in
   * @return number of bytes written
   *
   */
  int TcpSocket::Write(std::string msg_in) {
    size_t bytes_sent;       // total bytes sent
    int    this_write;       // bytes sent with this write()
    const char* buf = msg_in.c_str();
    bytes_sent = 0;

    while (bytes_sent < msg_in.length()) {
      do
        this_write = write(this->fd, buf, msg_in.length() - bytes_sent);
      while ((this_write < 0) && (errno == EINTR));
      if (this_write <= 0) return this_write;
      bytes_sent += this_write;
      buf += this_write;
    }
    return bytes_sent;
  }
  /**************** Network::TcpSocket::Write *********************************/


  /**************** Network::TcpSocket::Read **********************************/
  /**
   * @fn     Read
   * @brief  read data from connected socket
   * @param  buf, pointer to buffer
   * @param  count, number of bytes to read
   * @return number of bytes read or -1 on error
   *
   * If data not immediately available then wait for up to POLLTIMEOUT
   *
   */
  int TcpSocket::Read(void* buf, size_t count) {
    std::string function = "Network::TcpSocket::Read";
    std::stringstream message;
    int nread;

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    while ( ( nread = read( this->fd, buf, count ) ) < 0 ) {
      if ( errno != EAGAIN ) {
        message << "ERROR reading data on fd " << this->fd << ": " << strerror(errno);
        logwrite( function, message.str() );
        break;
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > POLLTIMEOUT ) {
        message << "ERROR: timeout waiting for data on fd " << this->fd;
        logwrite( function, message.str() );
        break;
      }
    }
    return( nread );
  }
  /**************** Network::TcpSocket::Read **********************************/


  /**************** Network::TcpSocket::Bytes_ready ***************************/
  /**
   * @fn     Bytes_ready
   * @brief  get the number of bytes available on the socket file descriptor this->fd
   * @param  buf, pointer to buffer
   * @param  count, number of bytes to read
   * @return number of bytes read
   *
   */
  int TcpSocket::Bytes_ready() {
    int bytesready=-1;
    if ( ioctl(this->fd, FIONREAD, &bytesready) < 0 ) {
      perror("(Network::TcpSocket::Bytes_ready) ioctl error");
    }
    return(bytesready);
  }
  /**************** Network::TcpSocket::Bytes_ready ***************************/

}
