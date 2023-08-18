/**
 * @file    logentry.cpp
 * @brief   logentry functions
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * Write time-stamped entries to log file using logwrite(function, message)
 * where function is a string representing the calling function and
 * message is an arbitrary string.
 *
 */

#include "logentry.h"

std::mutex loglock;            /// mutex to protect from multiple access
std::ofstream filestream;      /// IO stream class
unsigned int nextday = 86410;  /// number of seconds until a new day
bool to_stderr = true;         /// write to stderr


/***** init_log ***************************************************************/
/**
 * @brief      initializes the logging
 * @details    this function is overloaded
 * @param[in]  logpath    string is the path for log file
 * @param[in]  name       string is the name of the log file in logpath
 * @return     0=success, 1=error
 *
 * Opens an ofstream to the specified logfile, "logpath/name_YYYYMMDD.log"
 * where logpath and name are passed in as parameters.
 *
 * This version sets stderr_in to true (assumes logging to stderr).
 *
 */
long init_log( std::string logpath, std::string name ) {
  return init_log( logpath, name, true );
}


/***** init_log ***************************************************************/
/**
 * @brief      initializes the logging
 * @details    this function is overloaded
 * @param[in]  logpath    string is the path for log file
 * @param[in]  name       string is the name of the log file in logpath
 * @param[in]  stderr_in  true to log also to stderr
 * @return     0=success, 1=error
 *
 * Opens an ofstream to the specified logfile, "logpath/name_YYYYMMDD.log"
 * where logpath and name are passed in as parameters.
 *
 */
long init_log( std::string logpath, std::string name, bool stderr_in ) {
  std::string function = "init_log";
  std::stringstream filename;
  std::stringstream message;
  int year, mon, mday, hour, min, sec, usec;
  long error = 0;
  to_stderr = stderr_in;

  if ( ( error = get_time( year, mon, mday, hour, min, sec, usec ) ) ) return error;

  // assemble log file name from the passed-in arguments and current date
  //
  filename << logpath << "/" << name << "_" << std::setfill('0')
                      << std::setw(4) << year
                      << std::setw(2) << mon
                      << std::setw(2) << mday << ".log";

  // number of seconds until a new day
  //
  nextday = (unsigned int)(86410 - hour*3600 - min*60 - sec);

  // open the log file stream for append
  //
  try {
    filestream.open(filename.str(), std::ios_base::app);
  }
  catch(...) {
    message << "ERROR: opening log file " << filename.str() << ": " << std::strerror(errno);
    logwrite(function, message.str());
    return 1;
  }
  if ( ! filestream.is_open() ) {
    message << "ERROR: log file " << filename.str() << " not open";
    logwrite(function, message.str());
    return 1;
  }

  return 0;
}
/***** init_log ***************************************************************/


/***** close_log **************************************************************/
/**
 * @brief      closes the logfile stream
 *
 * Call this from the main class deconstructor to clean up the log file.
 *
 */
void close_log() {
  if (filestream.is_open() == true) {
    filestream.flush();
    filestream.close();
  }
}
/***** close_log **************************************************************/


/***** logwrite ***************************************************************/
/**
 * @brief      create a log file entry
 * @param[in]  function  string containing the Namespace::Class::function
 * @param[in]  message   string to log
 *
 * Create a time-stamped entry in the log file in the form of:
 * YYYY-MM-DDTHH:MM:SS.ssssss (function) message\n
 *
 * This function can also be used for logging to stderr, even if the
 * log filestream isn't open.
 *
 */
void logwrite(std::string function, std::string message) {
  std::stringstream logmsg;
  std::string timestamp = get_timestamp();       // get the current time (defined in utilities.h)

  std::lock_guard<std::mutex> lock(loglock);     // lock mutex to protect from multiple access

  logmsg << timestamp << "  (" << function << ") " << message << "\n";

  if (filestream.is_open()) {
    filestream << logmsg.str();                  // send to the file stream (if open)
    filestream.flush();
  }
  if ( to_stderr ) std::cerr << logmsg.str();    // send to standard error if requested
}
/***** logwrite ***************************************************************/

