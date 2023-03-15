/** ---------------------------------------------------------------------------
 * @file     slitd.h
 * @brief    slit daemon include file
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef SLITD_H
#define SLITD_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <csignal>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <climits>

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "build_date.h"
#include "daemonize.h"
#include "slit_server.h"

#define  N_THREADS    10          ///< total number of threads spawned by daemon, one for blocking and the rest for non-blocking
#define  CONN_TIMEOUT 3000        ///< incoming (non-blocking) connection timeout in milliseconds

Slit::Server slitd;               ///< global Slit::Server object so that the main daemon can access the namespace

void signal_handler(int signo);   ///< handles ctrl-C
int main(int argc, char **argv);  ///< the main function

#endif
