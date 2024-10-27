//
// Simple listener.cpp program for UDP multicast
//
// Adapted from:
// http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html
//
// Changes:
// * Compiles for Linux
// * Takes the port and group on the command line
// * adopted c++ idiomatic practices
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include "utilities.h"  // for cmdOption

constexpr size_t MSGBUFSIZE=256;

void usage(char* exe) {
  std::cout << "usage:\n"
            << exe << " <group> <port> [ -f,--filter | -w,--wait <message> ]\n"
            << "   where <group> <port> are the multicast group and port to listen to\n"
            << "   and options -f,--filter <message> will filter and display only lines,\n"
            << "   containing <message> and -w,--wait <message> will block until <message>\n"
            << "   is heard, then return (stop listening).\n\n";
  return;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
      usage(argv[0]);
      return 1;
    }

    char *group = argv[1]; // e.g. 239.255.255.250 for SSDP
    int port;
    try {
      port = std::stoi(argv[2]);
      if ( port < 1 ) throw std::out_of_range("invalid port number");
    }
    catch (...) {
      std::cerr << "invalid port number\n";
      return 1;
    }

    // display usage on request for help
    //
    if ( cmdOptionExists(argv, argv+argc, "-h") ||
         cmdOptionExists(argv, argv+argc, "--help" ) ) {
      usage(argv[0]);
      return 1;
    }

    char *filterstr = nullptr;
    char *waitstr = nullptr;

    // -f or --filter <message> will set filterstr
    // and when set, only those broadcast messages containing
    // <message> will be displayed.
    //
    if ( cmdOptionExists( argv, argv+argc, "-f" ) ) {
      filterstr = getCmdOption( argv, argv+argc, "-f" );
    }
    else
    if ( cmdOptionExists( argv, argv+argc, "--filter" ) ) {
      filterstr = getCmdOption( argv, argv+argc, "--filter" );
    }
    else
    // -w or --wait <message> will set waitstr and when set,
    // nothing will be displayed until a broadcast message
    // contains <message>, at which point it will print it and
    // return (useful for scripting).
    //
    if ( cmdOptionExists( argv, argv+argc, "-w" ) ) {
      waitstr = getCmdOption( argv, argv+argc, "-w" );
    }
    else
    if ( cmdOptionExists( argv, argv+argc, "--wait" ) ) {
      waitstr = getCmdOption( argv, argv+argc, "--wait" );
    }

    // create what looks like an ordinary UDP socket
    //
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        std::cerr << "socket: " << std::strerror(errno) << "\n";
        return 1;
    }

    // allow multiple sockets to use the same PORT number
    //
    u_int yes = 1;
    if (
        setsockopt(
            fd, SOL_SOCKET, SO_REUSEADDR, (char *) &yes, sizeof(yes)
        ) < 0
    ) {
        std::cerr << "Reusing ADDR: " << std::strerror(errno) << "\n";
        return 1;
    }

    // set up destination address
    //
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    addr.sin_port = htons(port);

    // bind to receive address
    //
    if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        std::cerr << "bind: " << std::strerror(errno) << "\n";
        return 1;
    }

    // use setsockopt() to request that the kernel join a multicast group
    //
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (
        setsockopt(
            fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &mreq, sizeof(mreq)
        ) < 0
    ) {
        std::cerr << "setsockopt: " << std::strerror(errno) << "\n";
        return 1;
    }

    // now just enter a read-print loop
    //
    while (1) {
        char msgbuf[MSGBUFSIZE];
        socklen_t addrlen = sizeof(addr);
        int nbytes = recvfrom(
            fd,
            msgbuf,
            MSGBUFSIZE,
            0,
            (struct sockaddr *) &addr,
            &addrlen
        );
        if (nbytes < 0) {
            std::cerr << "recvfrom: " << std::strerror(errno) << "\n";
            return 1;
        }
        msgbuf[nbytes] = '\0';

        // if the msgbuf contains waitstr then print that and stop
        //
        if (waitstr != nullptr) {
         if (strstr(msgbuf, waitstr) != nullptr) {
          std::cout << msgbuf << "\n";
          return 0;
         }
        }
        else
        // if the msgbug contains filterstr then print only that
        // and keep going
        //
        if (filterstr != nullptr) {
         if (strstr(msgbuf, filterstr) != nullptr) {
          std::cout << msgbuf << "\n";
         }
        }
        // otherwise print everything
        else std::cout << msgbuf << "\n";
    }

    return 0;
}
