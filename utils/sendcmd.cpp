/**
 * @file    utils/sendcmd.cpp
 * @brief   TCP command sender
 * @author  David Hale <dhale@caltech.edu>
 */
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>

static constexpr size_t RESPONSE_BUFSIZE = 8192;

static void usage(const char* exe) {
  std::cout << "usage: " << exe
            << " [-h hostname] [-p port] [-t timeout] [-m mode] command\n";
}

int main(int argc, char* argv[]) {
  std::string hostname;
  std::string message;
  int port    = 0;
  int timeout = 10;
  int mode    = 0;   // 0: wait for response, 1: fire and forget

  try {
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg.size() == 2 && arg[0] == '-') {
        if (i + 1 >= argc) { usage(argv[0]); return 0; }
        switch (arg[1]) {
          case 'h': hostname = argv[++i]; break;
          case 'p': port    = std::stoi(argv[++i]); break;
          case 't': timeout = std::stoi(argv[++i]); break;
          case 'm': mode    = std::stoi(argv[++i]); break;
          default:  usage(argv[0]); return 0;
        }
      }
      else {
        message = arg;
      }
    }
  }
  catch (const std::exception& ex) {
    std::cerr << "ERROR invalid argument: " << ex.what() << "\n";
    return 1;
  }

  if (hostname.empty() || port <= 0 || message.empty()) {
    usage(argv[0]);
    return 1;
  }

  struct addrinfo hints{};
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo* res = nullptr;
  const int gai_ret = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &res);
  if (gai_ret != 0) {
    std::cerr << "ERROR getting host: " << gai_strerror(gai_ret) << "\n";
    return 1;
  }

  int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sock < 0) {
    std::cerr << "ERROR " << errno << " creating socket\n";
    freeaddrinfo(res);
    return -errno;
  }

  if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
    std::cerr << "ERROR " << errno << " setting socket\n";
    close(sock);
    freeaddrinfo(res);
    return -errno;
  }

  const auto tstart = std::chrono::steady_clock::now();
  auto elapsed_sec = [&]() {
    return std::chrono::duration_cast<std::chrono::seconds>(
           std::chrono::steady_clock::now() - tstart).count();
  };

  if (connect(sock, res->ai_addr, res->ai_addrlen) == -1 && errno != EINPROGRESS) {
      std::cerr << "ERROR " << errno << " connecting\n";
      close(sock);
      freeaddrinfo(res);
      return -errno;
  }
  freeaddrinfo(res);

  message += '\n';

  ssize_t nwrite;
  while ((nwrite = write(sock, message.c_str(), message.size())) < 0) {
    if (errno != EAGAIN) {
      std::cerr << "ERROR " << errno << " writing message\n";
      close(sock);
      return -errno;
    }
    if (elapsed_sec() >= timeout) {
      std::cerr << "ERROR " << ETIME << " TIMEOUT\n";
      close(sock);
      return -ETIME;
    }
  }

  if (mode == 0) {
    char response[RESPONSE_BUFSIZE];
    ssize_t nread;
    while ((nread = read(sock, response, RESPONSE_BUFSIZE - 1)) < 0) {
      if (errno != EAGAIN) {
        std::cerr << "(sendcmd) ERROR " << errno << " reading\n";
        close(sock);
        return -errno;
      }
      usleep(10000);
      if (elapsed_sec() >= timeout) {
        std::cerr << "ERROR " << ETIME << " TIMEOUT\n";
        close(sock);
        return -ETIME;
      }
    }
    response[nread] = '\0';
    std::cout << response << "\n";
  }
  else {
    std::cout << "cmd_sent\n";
  }

  return close(sock);
}
