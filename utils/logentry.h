/** ---------------------------------------------------------------------------
 * @fn       logentry.h
 * @brief    include file for logging functions
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <thread>
#include <ctime>
#include "utilities.h"

extern unsigned int nextday;                               /// number of seconds until the next day is a global

long init_log( std::string logpath, std::string name );    /// initialize the logging system
void close_log();                                          /// close the log file stream
void logwrite(std::string function, std::string message);  /// create a time-stamped log entry "message" from "function"

#endif
