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

namespace Network {

  const long NO_ERROR = 0;
  const long ERROR = 1;
  const long TOUT = 3;

  /***** Network::UdpSocket::UdpSocket ****************************************/
  /**
   * @brief      UdpSocket class constructor
   * @param[in]  port_in   port on which server will multicast datagrams
   * @param[in]  group_in  multicast group
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
  /***** Network::UdpSocket::UdpSocket ****************************************/


  /***** Network::UdpSocket::UdpSocket ****************************************/
  /**
   * @brief      default UdpSocket class constructor
   *
   */
  UdpSocket::UdpSocket() {
    this->fd = -1;
    this->port = -1;
    this->group.clear();
    this->service_running = false;
  }
  /***** Network::UdpSocket::UdpSocket ****************************************/


  /***** Network::UdpSocket::UdpSocket ****************************************/
  /**
   * @brief      UdpSocket class copy constructor
   * @param[in]  obj  reference to class object
   *
   */
  UdpSocket::UdpSocket(const UdpSocket &obj) {
    fd = obj.fd;
    port = obj.port;
    group = obj.group;
    service_running = obj.service_running;
  }
  /***** Network::UdpSocket::UdpSocket ****************************************/


  /***** Network::UdpSocket::~UdpSocket ***************************************/
  /**
   * @brief      UdpSocket class deconstructor
   *
   */
  UdpSocket::~UdpSocket() {
    this->Close();
  };
  /***** Network::UdpSocket::~UdpSocket ***************************************/


  /***** Network::UdpSocket::Create *******************************************/
  /**
   * @brief      create a UDP multi-cast socket
   * @return     0 on success, -1 on error, 1 to indicate user-requested disable
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
  /***** Network::UdpSocket::Create *******************************************/


  /***** Network::UdpSocket::Send *********************************************/
  /**
   * @brief      transmit the message to the UDP socket
   * @return     0 on success, -1 on error
   *
   */
  int UdpSocket::Send(std::string message) {
    std::string function = "Network::UdpSocket::Send";
    std::stringstream errstm;
    int nbytes;

    if ( !this->is_running() ) return 0;  // silently do nothing if the UDP multicast socket isn't running

    if ( ( nbytes = sendto( this->fd, message.c_str(), (size_t)message.length(), 0, 
                            (struct sockaddr*) &this->addr, (socklen_t)sizeof(this->addr) ) ) < 0 ) {
      errstm << "error " << errno << " calling sendto: " << strerror(errno);
      logwrite(function, errstm.str());
      return -1;
    }

    return 0;
  }
  /***** Network::UdpSocket::Send *********************************************/


  /***** Network::UdpSocket::Listener *****************************************/
  /**
   * @brief      creates a UDP listener, returns a file descriptor
   * @return     fd on success, -1 on error
   *
   */
  int UdpSocket::Listener( ) {
    std::string function = "Network::UdpSocket::Listener";
    std::stringstream message;

    // don't create more than one UDP multicast socket
    //
    if ( this->service_running ) {
      logwrite( function, "ERROR: service already running" );
      return -1;
    }

    // don't do anything if the ASYNCGROUP is not initialized
    //
    if ( this->group.empty() ) {
      logwrite( function, "ERROR: ASYNCGROUP not initialized. Cannot create socket" );
      return -1;
    }

    // the user can set ASYNCGROUP=none to disable the async status message port
    //
    try {
      std::transform( this->group.begin(), this->group.end(), this->group.begin(), ::toupper );    // make uppercase
    }
    catch (...) {
      logwrite( function, "error converting ASYNCGROUP to uppercase" );
      return -1;
    }
    if ( this->group == "NONE" ) {
      logwrite( function, "ASYNCGROUP=none. UDP multicast socket disabled." );
      return 1;
    }

    // now that there is a group, check that the port is initialized
    //
    if ( this->port < 0 ) {
      logwrite( function, "ERROR: ASYNCPORT not initialized. Cannot create socket" );
      return -1;
    }

    // now that there is a group and port, create the socket
    //
    if ( ( this->fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
      message << "error " << errno << " creating socket: " << strerror( errno );
      logwrite(function, message.str());
      return(-1);
    }

    // allow multiple sockets to use the same PORT number
    //
    u_int yes = 1;
    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes) ) < 0 ) {
      message << "ERROR: reusing ADDR failed: " << strerror( errno );
      logwrite( function, message.str() );
      return -1;
    }

    // set up the source address
    //
    memset( &this->addr, 0, sizeof( this->addr ) );
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = htonl( INADDR_ANY ); // differs from sender
    this->addr.sin_port = htons( this->port );

    // bind to receive address
    //
    if ( bind( this->fd, (struct sockaddr*) &this->addr, sizeof(this->addr) ) < 0 ) {
      message << "ERROR binding to receive address: " << strerror( errno );
      logwrite( function, message.str() );
      return -1;
    }

    // use setsockopt() to request that the kernel join a multicast group
    //
    this->mreq.imr_multiaddr.s_addr = inet_addr( this->group.c_str() );
    this->mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if ( setsockopt( fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &this->mreq, sizeof(this->mreq) ) < 0 ) {
      message << "ERROR joining multicast group: " << strerror( errno );
      logwrite( function, message.str() );
      return -1;
    }

#ifdef LOGLEVEL_DEBUG
    message << "created UDP listening socket on fd " << this->fd;
    logwrite( function, message.str() );
#endif

    return( this->fd );
  }
  /***** Network::UdpSocket::Listener *****************************************/


  /***** Network::UdpSocket::Receive ******************************************/
  /**
   * @brief      receive a UDP message from the Listener fd
   * @param[out] message  reference to string to contain the message read
   * @return     number of bytes received
   *
   */
  int UdpSocket::Receive( std::string &message ) {
    char msgbuf[ UDPMSGLEN ];
    socklen_t addrlen = sizeof( this->addr );
    int nbytes = recvfrom ( this->fd,
                            msgbuf,
                            UDPMSGLEN,
                            0,
                            (struct sockaddr *) &this->addr,
                            &addrlen
                          );
    msgbuf[ nbytes ] = '\0';

    std::string msg( msgbuf, nbytes );
    message = msg;

    return nbytes;
  }
  /***** Network::UdpSocket::Receive ******************************************/


  /***** Network::UdpSocket::Close ********************************************/
  /**
   * @brief      close the UDP socket connection
   * @return     0 on success, -1 on error
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
  /***** Network::UdpSocket::Close ********************************************/


  /***** Network::TcpSocket::TcpSocket ****************************************/
  /**
   * @brief      TcpSocket class constructor
   * @param[in]  port_in    port which server will listen on
   * @param[in]  block_in   true|false -- will the connection be blocking?
   * @param[in]  totime_in  timeout time for poll, in msec
   * @param[in]  id_in      ID number (for keeping track of threads)
   *
   * Use this to construct a server's listening socket object
   *
   */
  TcpSocket::TcpSocket(int port_in, bool block_in, int totime_in, int id_in) {
    this->port = port_in;
    this->blocking = block_in;
    this->asyncflag = false;
    this->totime = totime_in;
    this->id = id_in;
    this->fd = -1;
    this->listenfd = -1;
    this->addrs = nullptr;
    this->connection_open = false;
  };
  /***** Network::TcpSocket::TcpSocket ****************************************/


  /***** Network::TcpSocket::TcpSocket ****************************************/
  /**
   * @brief      TcpSocket class constructor
   * @param[in]  port_in    port which server will listen on
   * @param[in]  block_in   true|false -- will the connection be blocking?
   * @param[in]  async_in   true|false -- will the connection be asynchronous?
   * @param[in]  totime_in  timeout time for poll, in msec
   * @param[in]  id_in      ID number (for keeping track of threads)
   *
   * Use this to construct a server's listening socket object
   *
   */
  TcpSocket::TcpSocket(int port_in, bool block_in, bool async_in, int totime_in, int id_in) {
    this->port = port_in;
    this->blocking = block_in;
    this->asyncflag = async_in;
    this->totime = totime_in;
    this->id = id_in;
    this->fd = -1;
    this->listenfd = -1;
    this->addrs = nullptr;
    this->connection_open = false;
  };
  /***** Network::TcpSocket::TcpSocket ****************************************/


  /***** Network::TcpSocket::TcpSocket ****************************************/
  /**
   * @brief      TcpSocket class constructor
   * @param[in]  host_in    host name
   * @param[in]  port_in    port which server will listen on
   * @param[in]  block_in   true|false -- will the connection be blocking?
   * @param[in]  async_in   true|false -- will the connection be asynchronous?
   * @param[in]  totime_in  timeout time for poll, in msec
   * @param[in]  id_in      ID number (for keeping track of threads)
   *
   * Use this to construct a server's listening socket object
   *
   */
  TcpSocket::TcpSocket( std::string host_in, int port_in, bool block_in, bool async_in, int totime_in, int id_in) {
    this->host = host_in;
    this->port = port_in;
    this->blocking = block_in;
    this->asyncflag = async_in;
    this->totime = totime_in;
    this->id = id_in;
    this->fd = -1;
    this->listenfd = -1;
    this->addrs = nullptr;
    this->connection_open = false;
  };
  /***** Network::TcpSocket::TcpSocket ****************************************/


  /***** Network::TcpSocket::TcpSocket ****************************************/
  /**
   * @brief      TcpSocket class constructor
   * @param[in]  host  hostname to which client will connect
   * @param[in]  port  port to which client will connect
   *
   * Use this to construct a client object
   *
   */
  TcpSocket::TcpSocket( std::string host, int port ) {
    this->host = host;
    this->port = port;
    this->totime = POLLTIMEOUT;    /// default Poll timeout in msec
    this->fd = -1;
    this->addrs = nullptr;
    this->connection_open = false;
  }
  /***** Network::TcpSocket::TcpSocket ****************************************/


  /***** Network::TcpSocket::TcpSocket ****************************************/
  /**
   * @brief      default TcpSocket class constructor
   *
   */
  TcpSocket::TcpSocket() {
    this->port = -1;
    this->blocking = false;
    this->asyncflag = false;
    this->totime = POLLTIMEOUT;    /// default Poll timeout in msec
    this->id = -1;
    this->fd = -1;
    this->listenfd = -1;
    this->addrs = nullptr;
    this->connection_open = false;
  };
  /***** Network::TcpSocket::TcpSocket ****************************************/


  /***** Network::TcpSocket::TcpSocket ****************************************/
  /**
   * @brief      TcpSocket class copy constructor
   * @param[in]  obj  reference to class object
   *
   */
  TcpSocket::TcpSocket(const TcpSocket &obj) {
    port = obj.port;
    blocking = obj.blocking;
    asyncflag = obj.asyncflag;
    totime = obj.totime;
    id = obj.id;
    fd = obj.fd;
    listenfd = obj.listenfd;
    host = obj.host;
    connection_open = obj.connection_open;

    // Perform a deep copy of addrs
    //
    if ( obj.addrs != nullptr ) {
      struct addrinfo *addrCopy = nullptr;
      struct addrinfo *current = nullptr;
      struct addrinfo *prev = nullptr;
      try {
        for ( struct addrinfo *p = obj.addrs; p != nullptr; p = p->ai_next ) {
          current = new struct addrinfo;
          if ( current == nullptr ) {
            // Clean up previous allocations before throwing an exception
            while ( addrCopy != nullptr ) {
              struct addrinfo *temp = addrCopy;
              addrCopy = addrCopy->ai_next;
              delete temp;
            }
            throw std::bad_alloc(); // or another appropriate exception
          }
          memcpy( current, p, sizeof( struct addrinfo ) );
          if ( prev != nullptr ) {
            prev->ai_next = current;
          }
          else {
            addrCopy = current;
          }
          prev = current;
        }
      }
      catch ( ... ) { // Clean up in case of exception
        while ( addrCopy != nullptr ) {
          struct addrinfo *temp = addrCopy;
          addrCopy = addrCopy->ai_next;
          delete temp;
        }
        throw; // Re-throw the exception
      }
      addrs = addrCopy;
    }
    else {
      addrs = nullptr;
    }
  };
  /***** Network::TcpSocket::TcpSocket ****************************************/


  /***** Network::TcpSocket::Accept *******************************************/
  /**
   * @brief      creates a new connected socket for pending connection
   * @return     fd  file descriptor of new connected socket file descriptor, or -1 on error
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
  /***** Network::TcpSocket::Accept *******************************************/


  /***** Network::TcpSocket::Listen *******************************************/
  /**
   * @brief      create a TCP listening socket
   * @return     listenfd  listening file descriptor, or -1 on error
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
      errstm << "error " << errno << " binding to fd " << this->listenfd << " on port " << this->port << ": " << strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    // start listening
    //
    if (listen(this->listenfd, LISTENQ)!=0) {
      errstm << "error " << errno << " listening to fd " << this->listenfd << " on port " << this->port << ": " << strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    return(this->listenfd);
  }
  /***** Network::TcpSocket::Listen *******************************************/


  /***** Network::TcpSocket::Poll *********************************************/
  /**
   * @brief      polls a single file descriptor to wait for incoming data to read
   * @return     0 on timeout, -1 on error, a non-negative value on success
   *
   * This function is overloaded. One version accepts a timeout to be passed in,
   * and the other version takes no argument and uses the default timeout that
   * was set for the class object.
   *
   */
  int TcpSocket::Poll() {                /// uses default timeout
    return( this->Poll( this->totime ) );
  }
  /***** Network::TcpSocket::Poll *********************************************/


  /***** Network::TcpSocket::Poll *********************************************/
  /**
   * @brief      polls a single file descriptor to wait for incoming data to read
   * @param[in]  timeout  number of milliseconds that poll() should block waiting for a file descriptor to become ready
   * @return     0 on timeout, -1 on error, a non-negative value on success
   *
   * This function is overloaded. One version accepts a timeout to be passed in,
   * and the other version takes no argument and uses the default timeout that
   * was set for the class object.
   *
   */
  int TcpSocket::Poll( int timeout ) {
    std::string function = "Network::TcpSocket::Poll";
    std::stringstream message;
    struct pollfd poll_struct;
    poll_struct.events = POLLIN;
    poll_struct.fd     = this->fd;

    int ret = poll( &poll_struct, 1, timeout );
    short revents = poll_struct.revents;

    // Look for some events:
    //  POLLHUP (hang-up)           Remote side has closed their connection
    //  POLLERR (error)             An error condition on the file descriptor
    //  POLLNVAL (invalid request)  Invalid request such as polling a fd that isn't open
    //
    if ( ( revents & POLLHUP ) || ( revents & POLLERR) || ( revents & POLLNVAL ) ) {
      message.str(""); message << ( revents & POLLHUP  ? "POLLHUP "  : "" )
                               << ( revents & POLLERR  ? "POLLERR "  : "" )
                               << ( revents & POLLNVAL ? "POLLNVAL " : "" )
                               << "recevied: closing socket " << this->host << "/" << this->port << " on fd " << this->fd;
      logwrite( function, message.str() );
      this->Close();
      ret = -1;
    }

    return( ret );
  }
  /***** Network::TcpSocket::Poll *********************************************/


  /***** Network::TcpSocket::Connect ******************************************/
  /**
   * @brief      connect to this->host on this->port
   * @return     0 on success, -1 on error
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

    struct addrinfo hints = {0};       /// destination for getaddrinfo

    hints.ai_family = AF_INET;         /// IPv4 address only
    hints.ai_socktype = SOCK_STREAM ;  /// non-blocking, connection-based socket
    hints.ai_protocol = IPPROTO_TCP;   /// TCP socket
    hints.ai_flags = AI_NUMERICSERV;   /// numeric port number

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

    if (fcntl(this->fd, F_SETFL, flags) < 0) {
      errstm << "error " << errno << " setting socket file descriptor flags: " << std::strerror(errno);
      logwrite(function, errstm.str());
      return(-1);
    }

    this->connection_open = (this->fd >= 0 ? true : false);

    return (this->fd >= 0 ? 0 : -1);
  }
  /***** Network::TcpSocket::Connect ******************************************/


  /***** Network::TcpSocket::Close ********************************************/
  /**
   * @brief      close a socket connection and free memory allocated by getaddrinfo()
   * @return     0 on success, -1 on error
   *
   */
  int TcpSocket::Close() {
    std::string function = "Network::TcpSocket::Close";
    std::stringstream message;
    int error = -1;

    if (this->fd >= 0) {               // if the file descriptor is valid
      if (close(this->fd) == 0) {      // then close it
#ifdef LOGLEVEL_DEBUG
//      message.str(""); message << "[DEBUG] connection to " << this->host << "/" << this->port << " on fd " << this->fd << " closed";
//      logwrite( function, message.str() );
#endif
        error = 0;
        this->fd = -1;
      }
      else {
        message.str(""); message << "ERROR closing fd " << this->fd << " on port " << this->port
                                 << " returned " << errno << ": " << strerror(errno);
        logwrite( function, message.str() );
        error = -1;                    // error closing file descriptor
     }
    }
    else {
      error = 0;                       // didn't start with a valid file descriptor
    }

    if (this->addrs != nullptr) {      // free memory allocated by getaddrinfo()
      freeaddrinfo(this->addrs);
      this->addrs = nullptr;
    }

    this->connection_open = false;     // clear the connection_open flag

    return error;
  }
  /***** Network::TcpSocket::Close ********************************************/


  /***** Network::TcpSocket::Write ********************************************/
  /**
   * @brief      write data to a socket
   * @param[in]  msg_in  string to write
   * @return     number of bytes written
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
  /***** Network::TcpSocket::Write ********************************************/


  /***** Network::TcpSocket::Write ********************************************/
  /**
   * @brief      write a single char to a socket
   * @param[in]  c  const char pointer
   * @return     number of bytes written or -1 on error
   *
   */
  ssize_t TcpSocket::Write( const char* c ) {
    if (!c) {                                // need non-null char
      logwrite( "Network::TcpSocket::Write", "ERROR requested write nullptr" );
      return -1;
    }
    ssize_t ret;
    if ( this->fd < 0 ) {                    // need valid file descriptor
      logwrite( "Network::TcpSocket::Write", "ERROR invalid file descriptor" );
      ret=-1;
    }
    else {
      ret = write( this->fd, (void*)c, 1 );  // write the single char
      if (ret<0) {
        std::stringstream message;
        message << "ERROR write \"" << c << "\" to fd " << this->fd << " failed: " << strerror(errno);
        logwrite( "Network::TcpSocket::Write", message.str() );
      }
    }
    return ret;                              // return value of write() or -1
  }
  /***** Network::TcpSocket::Write ********************************************/


  /***** Network::TcpSocket::Read *********************************************/
  /**
   * @brief      read data from connected socket
   * @param[in]  buf    pointer to buffer
   * @param[in]  count  number of bytes to read
   * @return     number of bytes read or -1 on error
   *
   * If data not immediately available then wait for up to POLLTIMEOUT
   *
   * This function is overloaded; this version accepts a pointer to a
   * buffer and the number of bytes to read.
   *
   */
  int TcpSocket::Read(void* buf, const size_t count) {
    std::string function = "Network::TcpSocket::Read[cbuf]";
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
  /***** Network::TcpSocket::Read *********************************************/


  /***** Network::TcpSocket::Read *********************************************/
  /**
   * @brief      read data from connected socket until newline
   * @details    this calls the Read(retstring,term) with term set to newline
   * @param[out] retstring  reference to string to contain reply
   * @return     number of bytes read or -1 on error
   *
   * This function is overloaded
   *
   */
  int TcpSocket::Read( std::string &retstring ) {
    return this->Read( retstring, '\n' );
  }
  /***** Network::TcpSocket::Read *********************************************/


  /***** Network::TcpSocket::Read *********************************************/
  /**
   * @brief      read data from connected socket until terminating char
   * @param[out] retstring  reference to string to read in to
   * @param[in]  term       read until this terminating char is found
   * @return     number of bytes read or -1 on error or -2 on timeout
   *
   * If data not immediately available then wait for up to POLLTIMEOUT
   *
   * This function is overloaded; this version accepts a reference to a string
   * and a terminating char to read until. The terminating character remains
   * in the string.
   *
   */
  int TcpSocket::Read( std::string &retstring, const char &term ) {
    std::string function = "Network::TcpSocket::Read[term]";
    std::stringstream message;
    size_t bytesread=0;
    retstring.clear();                   // make sure the return string starts empty

    // get the time now for timeout purposes
    //
    auto tstart = std::chrono::steady_clock::now();

    while ( true ) {
      char charin;
      size_t nread = read( this->fd, &charin, 1 );  // read a byte at a time
      if ( nread<0 ) {
        message << "ERROR reading data on fd " << this->fd << ": " << strerror(errno);
        logwrite( function, message.str() );
        return -1;                       // indicates error
      }
      if ( nread == 0 ) {
#ifdef LOGLEVEL_DEBUG
        message << "[DEBUG] no data on socket " << this->host << ":" << this->port << " fd " << this->fd << ". closing connection";
        logwrite( function, message.str() );
#endif
        this->Close();
        return 0;                        // not an error
      }
      bytesread++;                       // keep count of total bytes read

      if ( charin == term ) {            // check for terminating character
        if ( bytesread > 1 ) {           // break only when terminated with that char
          break;
        }
      }
      retstring.push_back( charin );     // append the read-in character to the return string

      // get time now and check for timeout
      //
      auto tnow = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > POLLTIMEOUT ) {
        message << "ERROR: timeout waiting for data on fd " << this->fd;
        logwrite( function, message.str() );
        return -2;                       // indicates timeout
      }
    }

    return bytesread;                    // return bytes read
  }
  /***** Network::TcpSocket::Read *********************************************/


  /***** Network::TcpSocket::Read *********************************************/
  /**
   * @brief      read data from connected socket until an end string
   * @param[out] retstring  reference to string to read in to
   * @param[in]  endstr     read until this string is found
   * @return     number of bytes read or -1 on error
   *
   * If data not immediately available then wait for up to POLLTIMEOUT
   *
   * This function is overloaded; this version accepts a reference to a string
   * and an end string to read until.
   *
   */
  int TcpSocket::Read( std::string &retstring, const std::string &endstr ) {
    std::string function = "Network::TcpSocket::Read[endstr]";
    std::stringstream message;
    std::stringstream bufstream;
    int nread, bytesread=0;
    const int bufsz=8192;                // read buffer in chunks
    char buf[bufsz+1];
    memset(buf,'\0',bufsz+1);

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    while ( 1 ) {
      nread = read( this->fd, buf, bufsz );
      if ( nread<0 ) {
        message << "ERROR reading socket " << this->host << "/" << this->port << " on fd " << this->fd << ": " << strerror(errno);
        logwrite( function, message.str() );
        break;
      }
      if ( nread == 0 ) {
        message << "ERROR no data from socket " << this->host << "/" << this->port << " on fd " << this->fd << ": closing connection";
        logwrite( function, message.str() );
        this->Close();
        break;
      }
      bytesread += nread;                // keep count of total bytes read
      bufstream << buf;                  // build up return string from each byte read

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > POLLTIMEOUT ) {
        message << "ERROR: timeout waiting for data on fd " << this->fd;
        logwrite( function, message.str() );
        break;
      }

      // Break when the endstr is found in the buffer read
      //
      if ( bufstream.str().find( endstr ) != std::string::npos ) break;
    }

    retstring = bufstream.str();         // assign the assembled stream to the return string

    if ( nread <= 0 ) return( nread );   // return error
    else return( bytesread );            // or bytes read
  }
  /***** Network::TcpSocket::Read *********************************************/


  /***** Network::TcpSocket::Bytes_ready **************************************/
  /**
   * @brief      get the number of bytes available on the socket file descriptor this->fd
   * @return     number of bytes read
   *
   */
  int TcpSocket::Bytes_ready() {
    int bytesready=-1;
    if ( ioctl(this->fd, FIONREAD, &bytesready) < 0 ) {
      perror("(Network::TcpSocket::Bytes_ready) ioctl error");
    }
    return(bytesready);
  }
  /***** Network::TcpSocket::Bytes_ready **************************************/


  /***** Network::TcpSocket::Flush ********************************************/
  /**
   * @brief      flush a socket by reading until it's empty
   *
   */
  void TcpSocket::Flush() {
    struct pollfd poll_struct;
    poll_struct.events = POLLIN;
    poll_struct.fd     = this->fd;  // poll the current file descriptor

    poll( &poll_struct, 1, 1000 );  // poll up to 1 sec

    while ( true ) {
      char buf[1024];
      int len = recv( poll_struct.fd, buf, sizeof(buf), MSG_DONTWAIT );
      if ( len == -1 ) break;
    }
    return;
  }
  /***** Network::TcpSocket::Flush ********************************************/


  /***** Network::Interface::Interface ****************************************/
  /**
   * @brief      default Interface class constructor
   *
   */
  Interface::Interface( ) {
    this->name.clear();
    this->port = -1;
    this->term_write = '\n';
    this->term_read  = '\n';
    this->initialized = false;
  }
  /***** Network::Interface::Interface ****************************************/


  /***** Network::Interface::Interface ****************************************/
  /**
   * @brief      Interface class constructor
   * @details    when constructed without terminating chars, use newline
   * @param[in]  name   name of the device for info purposes only
   * @param[in]  host   hostname of the device
   * @param[in]  port   port number of the device
   *
   */
  Interface::Interface( std::string name, std::string host, int port ) {
    this->name = name;
    this->port = port;
    this->host = host;
    this->term_write = '\n';
    this->term_read  = '\n';
    this->initialized = true;
  }
  /***** Network::Interface::Interface ****************************************/


  /***** Network::Interface::Interface ****************************************/
  /**
   * @brief      Interface class constructor
   * @details    this constructor accepts terminating chars
   * @param[in]  name        name of the device for info purposes only
   * @param[in]  host        hostname of the device
   * @param[in]  port        port number of the device
   * @param[in]  term_write  send_command() adds this char on writes
   * @param[in]  term_read   send_command() looks for this char on reads (if reply requested)
   *
   */
  Interface::Interface( std::string name, std::string host, int port, char term_write, char term_read ) {
    this->name = name;
    this->port = port;
    this->host = host;
    this->term_write = term_write;
    this->term_read  = term_read;
    this->initialized = true;
  }
  /***** Network::Interface::Interface ****************************************/


  /***** Network::Interface::~Interface ***************************************/
  /**
   * @brief      Interface class destructor
   *
   */
  Interface::~Interface() {
  };
  /***** Network::Interface::~Interface ***************************************/


  /***** Network::Interface::open *********************************************/
  /**
   * @brief      open a connection to the device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "Network::Interface::open";
    std::stringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( this->sock.isconnected() ) {
      message.str(""); message << "socket connection already open to "
                               << this->sock.gethost() << ":" << this->sock.getport()
                               << " on fd " << this->sock.getfd() << " for " << this->name;
      logwrite( function, message.str() );
      return( NO_ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->sock = s;

    // Initialize connection to the device
    //
    message.str(""); message << "opening socket connection for " << this->name;
    logwrite( function, message.str() );

    if ( this->sock.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting socket for " << this->name;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // flush the buffer in case the device will write stuff upon opening the socket
    //
    this->sock.Flush();

    message.str(""); message << "socket connection to "
                             << this->sock.gethost() << ":" << this->sock.getport()
                             << " established on fd " << this->sock.getfd() << " for " << this->name;
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Network::Interface::open *********************************************/


  long Interface::reconnect() {
    logwrite("Network::Interface::reconnect","reconnecting");
    long error = this->close();
    std::this_thread::sleep_for( std::chrono::milliseconds(100) );
    if (error==NO_ERROR) error = this->open();
    return error;
  }
  /***** Network::Interface::close ********************************************/
  /**
   * @brief      close the connection to the device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    std::string function = "Network::Interface::close";
    std::stringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      message.str(""); message << "socket connection already closed for " << this->name;
      logwrite( function, message.str() );
      return( NO_ERROR );
    }

    long error = this->sock.Close();

    if ( error == NO_ERROR ) {
      message.str(""); message << "socket connection for " << this->name << " closed";
      logwrite( function, message.str() );
    }
    else {
      message.str(""); message << "ERROR closing socket connection for " << this->name;
      logwrite( function, message.str() );
    }

    return( error );
  }
  /***** Network::Interface::close ********************************************/


  /***** Network::Interface::send_command *************************************/
  /**
   * @brief      send specified command to the socket interface
   * @details    This function is overloaded. This version won't read a reply.
   * @param[in]  cmd        command to send
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string retstring="noreply";
    return this->send_command( cmd, retstring );
  }
  /***** Network::Interface::send_command *************************************/


  /***** Network::Interface::send_command *************************************/
  /**
   * @brief      send specified command to the socket interface and return reply
   * @details    This function is overloaded. This version returns the reply.
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply read back from device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring ) {
    return send_command( cmd, retstring, this->sock.polltimeout() );
  }
  long Interface::send_command( std::string cmd, std::string &retstring, int timeout ) {
    std::string function = "Network::Interface::send_command";
    std::stringstream message;
    std::string reply;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      message.str(""); message << "ERROR not connected to " << this->name;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // send the command
    //
    cmd += this->term_write;                       // add the terminating character for writes
    int written = this->sock.Write( cmd );         // write the command
    if ( written <= 0 ) {                          // return error if error writing to socket
      cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n' ), cmd.end());  // remove the newline for better logging
      message.str(""); message << "ERROR sending \"" << cmd << "\" to " << this->name;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Unless retstring has been pre-set with "noreply" by the overloaded function,
    // then try to read a reply.
    //
    bool needs_reply = ( retstring=="noreply" ? false : true );
    long error=NO_ERROR;

    if ( needs_reply ) {
      long retval=0;
      if ( ( retval=this->sock.Poll(timeout) ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "TIMEOUT on fd " << this->sock.getfd();
                           if (errno) { message << ": " << strerror(errno); }
                           message << ": timeout=" << timeout;
                           error = TOUT; }
        if ( retval <0 ) { message.str(""); message << "ERROR on fd " << this->sock.getfd();
                           if (errno) { message << ": " << strerror(errno); }
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
      }
      else
      if ( ( retval = this->sock.Read( reply, this->term_read ) ) < 0 ) {
        message.str(""); message << "ERROR: " << strerror( errno )
                                 << ": reading from " << this->sock.gethost() << "/" << this->sock.getport();
        logwrite( function, message.str() );
      }

//    command-response debugging aid to look for non-printing characters in reply
//    message.str(""); message << "[DEBUG] raw reply=0x";
//    for ( size_t i=0; i<10 && i<reply.length(); i++ ) {
//      message << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)reply[i] << " ";
//    }
//    message << " to cmd=" << cmd;
//    logwrite(function,message.str());

      // send back just the reply, stripped of any leading/trailing control characters
      //
      reply = strip_control_characters( reply );
    }

    retstring = reply;

    return( error );
  }
  /***** Network::Interface::send_command *************************************/

}
