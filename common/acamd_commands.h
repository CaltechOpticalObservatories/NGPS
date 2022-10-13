/**
 * @file    acamd_commands.h
 * @brief   the Acam::Server::doit() function listens for these commands.
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 *
 */

#ifndef ACAMD_COMMANDS_H
#define ACAMD_COMMANDS_H
const std::string ACAMD_FILTER = "filter";  ///< filter [ name ] to set or get the filter
const std::string ACAMD_COVER  = "cover";   ///< cover [ open|close ] to set or get the dust over
const std::string ACAMD_HOME   = "home";    ///< home all motors
const std::string ACAMD_ISHOME = "ishome";  ///< are all motors homed?
const std::string ACAMD_OPEN   = "open";    ///< open connection to all devices
const std::string ACAMD_ISOPEN = "isopen";  ///< close connection to all devices
#endif
