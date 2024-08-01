/** ---------------------------------------------------------------------------
 * @file     slicecamd.h
 * @brief    slicecam daemon include file
 * @details  defines the classes used by the slicecam daemon
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
#include "slicecam_server.h"


#define  CONN_TIMEOUT 3000        ///< incoming (non-blocking) connection timeout in milliseconds

void signal_handler(int signo);   ///< handles ctrl-C
int main(int argc, char **argv);  ///< the main function
