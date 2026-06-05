/**
 * @file    sd_notify.h
 * @brief   minimal systemd service-notification helper (no libsystemd dependency)
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * @details Sends service-readiness notifications to systemd via the socket named
 *          in the $NOTIFY_SOCKET environment variable, as set for units with
 *          Type=notify. If $NOTIFY_SOCKET is unset (e.g. the daemon was launched
 *          outside systemd, or under Type=simple), every call is a harmless
 *          no-op that returns false. This avoids linking libsystemd.
 *
 *          Call Daemon::sd_notify_ready() exactly once, right after the daemon's
 *          command/listening socket is up (NOT after hardware is opened) --
 *          "ready" must mean "ready to accept client commands".
 */

#ifndef SD_NOTIFY_H
#define SD_NOTIFY_H

#include <cstddef>      // offsetof
#include <cstdlib>      // getenv
#include <cstring>      // memset, strncpy, strlen
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>     // close

namespace Daemon {

  /***** Daemon::sd_notify ****************************************************/
  /**
   * @brief      send a raw notification string to systemd
   * @param[in]  state  notification payload, e.g. "READY=1\n"
   * @return     true if the message was sent, false otherwise (incl. no-op)
   *
   * No-op (returns false) when not running under systemd Type=notify, i.e. when
   * $NOTIFY_SOCKET is unset. Supports both path-based and abstract ('@') sockets.
   *
   */
  inline bool sd_notify( const std::string &state ) {
    // when systemd launches a service with notification enabled, it sets this env
    const char* path = std::getenv( "NOTIFY_SOCKET" );
    if ( path == nullptr || path[0] == '\0' ) return false;   // not under systemd notify

    // systemd uses either an abstract socket ("@/org/...") or a filesystem path.
    if ( path[0] != '/' && path[0] != '@' ) return false;     // unsupported address

    int fd = ::socket( AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0 );
    if ( fd < 0 ) return false;

    struct sockaddr_un addr;
    std::memset( &addr, 0, sizeof(addr) );
    addr.sun_family = AF_UNIX;

    socklen_t addrlen;
    if ( path[0] == '@' ) {
      // abstract namespace: leading NUL, then the name after the '@'
      addr.sun_path[0] = '\0';
      std::strncpy( addr.sun_path + 1, path + 1, sizeof(addr.sun_path) - 2 );
      addrlen = static_cast<socklen_t>( offsetof(struct sockaddr_un, sun_path)
                                        + 1 + std::strlen(path + 1) );
    }
    else {
      std::strncpy( addr.sun_path, path, sizeof(addr.sun_path) - 1 );
      addrlen = static_cast<socklen_t>( offsetof(struct sockaddr_un, sun_path)
                                        + std::strlen(addr.sun_path) );
    }

    // use global libc/posix sendto and close
    ssize_t n = ::sendto( fd, state.data(), state.size(), MSG_NOSIGNAL,
                          reinterpret_cast<struct sockaddr*>(&addr), addrlen );
    ::close( fd );
    return ( n == static_cast<ssize_t>( state.size() ) );
  }
  /***** Daemon::sd_notify ****************************************************/


  /** @brief  tell systemd the service is ready (Type=notify start completes)
   */
  inline bool sd_notify_ready() { return sd_notify( "READY=1\n" ); }


  /** @brief  tell systemd the service is shutting down
   */
  inline bool sd_notify_stopping() { return sd_notify( "STOPPING=1\n" ); }

}
#endif
