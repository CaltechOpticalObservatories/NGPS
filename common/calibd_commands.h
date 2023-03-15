/**
 * @file    calibd_commands.h
 * @brief   defines the commands accepted by the calib daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef CALIBD_COMMANDS_H
#define CALIBD_COMMANDS_H
const std::string CALIBD_OPEN = "open";      ///< open connection to calib
const std::string CALIBD_CLOSE = "close";    ///< close connection to calibd hardware
const std::string CALIBD_ISOPEN = "isopen";  ///< is calib open
#endif
