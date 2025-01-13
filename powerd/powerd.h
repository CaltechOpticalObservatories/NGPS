/** ---------------------------------------------------------------------------
 * @file     powerd.h
 * @brief    power daemon include file
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

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
#include "power_server.h"

#define  N_THREADS    10          ///< total number of threads spawned by daemon, one for blocking and the rest for non-blocking
#define  CONN_TIMEOUT 3000        ///< incoming (non-blocking) connection timeout in milliseconds

void signal_handler(int signo);   ///< handles ctrl-C
int main(int argc, char **argv);  ///< the main function
