/** ---------------------------------------------------------------------------
 * @file     tcsd.h
 * @brief    tcs daemon include file
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 * This defines the functions used by the global main daemon.
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
#include <condition_variable>
#include <cmath>
#include <ctgmath>

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "build_date.h"
#include "daemonize.h"
#include "tcs_server.h"

#define  CONN_TIMEOUT 3000        ///< incoming (non-blocking) connection timeout in milliseconds

void signal_handler(int signo);   ///< handles ctrl-C
int main(int argc, char **argv);  ///< the main function
