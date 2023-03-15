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
const std::string ACAMD_EXIT     = "exit";      ///< 
const std::string ACAMD_ECHO     = "echo";      ///< 
const std::string ACAMD_FILTER   = "filter";    ///< filter [ name ] to set or get the filter
const std::string ACAMD_COVER    = "cover";     ///< cover [ open|close ] to set or get the dust over
const std::string ACAMD_HOME     = "home";      ///< home all motors
const std::string ACAMD_ISHOME   = "ishome";    ///< are all motors homed?
const std::string ACAMD_OPEN     = "open";      ///< open connection to all devices
const std::string ACAMD_ISOPEN   = "isopen";    ///< close connection to all devices
const std::string ACAMD_CLOSE    = "close";     ///< close connection to all devices
const std::string ACAMD_CAMOPEN  = "camopen";   ///< open connection to camera
const std::string ACAMD_CAMCLOSE = "camclose";  ///< close connection to camera
const std::string ACAMD_ACQUIRE  = "acquire";   ///< the main acquire wrapper
const std::string ACAMD_INIT     = "init";      ///< 
const std::string ACAMD_SOLVE    = "solve";     ///< call the Python astrometry solver
const std::string ACAMD_CAMERASERVER_COORDS = "prepare";  ///< send coordinates to external camera server
const std::string ACAMD_CAMERASERVER_ACQUIRE = "cameraserveracquire";  ///< acquire an image from the acam camera server
const std::vector<std::string> ACAMD_SYNTAX = { 
                                                ACAMD_EXIT,
                                                ACAMD_ECHO,
                                                ACAMD_FILTER,
                                                ACAMD_COVER,
                                                ACAMD_HOME,
                                                ACAMD_ISHOME,
                                                ACAMD_OPEN,
                                                ACAMD_CLOSE,
                                                ACAMD_CAMOPEN,
                                                ACAMD_CAMCLOSE,
                                                ACAMD_ACQUIRE,
                                                ACAMD_INIT,
                                                ACAMD_SOLVE,
                                                ACAMD_CAMERASERVER_COORDS,
                                                ACAMD_CAMERASERVER_ACQUIRE
                                              };
#endif
