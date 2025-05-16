/**
 * @file    camerad.h
 * @brief   camera daemon include file
 * @author  David Hale <dhale@astro.caltech.edu>
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

#include "network.h"
#include "build_date.h"
#include "daemonize.h"
#include "camera_server.h"
#include "utilities.h"
#include "common.h"
#include "logentry.h"

//void signal_handler(int signo);   ///< handles ctrl-C and other signals
int main(int argc, char** argv);  ///< the main function
