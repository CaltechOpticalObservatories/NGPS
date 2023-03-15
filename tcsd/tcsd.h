/** ---------------------------------------------------------------------------
 * @file     tcsd.h
 * @brief    tcs daemon include file
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 * This defines the functions used by the global main daemon.
 *
 */

#ifndef TCSD_H
#define TCSD_H

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
#include <condition_variable>
#include <cmath>
#include <ctgmath>

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "build_date.h"
#include "daemonize.h"
#include "tcs_server.h"

#define  N_THREADS    10          ///< total number of threads spawned by sequencer, one for blocking and the rest for non-blocking
#define  CONN_TIMEOUT 3000        ///< incoming (non-blocking) connection timeout in milliseconds

TCS::Server tcsd;                 ///< global TCS::Server object so that the main daemon can access the namespace

void signal_handler(int signo);   ///< handles ctrl-C
int main(int argc, char **argv);  ///< the main function

#endif
