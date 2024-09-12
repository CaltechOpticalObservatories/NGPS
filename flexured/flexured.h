/** ---------------------------------------------------------------------------
 * @file     flexured.h
 * @brief    flexure daemon include file
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
#include "flexure_server.h"

#define  CONN_TIMEOUT 3000        ///< incoming (non-blocking) connection timeout in milliseconds

Flexure::Server flexured;         ///< global Flexure::Server object so that the main daemon can access the namespace

void signal_handler(int signo);   ///< handles ctrl-C
int main(int argc, char **argv);  ///< the main function
