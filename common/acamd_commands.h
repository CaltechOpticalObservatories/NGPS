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
const std::string ACAMD_ACQUIRE  = "acquire";   ///< *** the main acquire wrapper
const std::string ACAMD_BIN      = "bin";       ///< set/get camera binning
const std::string ACAMD_CAMERASERVER_COORDS = "prepare";  ///< *** send coordinates to external camera server
const std::string ACAMD_CLOSE    = "close";     ///< *** close connection to all devices
const std::string ACAMD_CONFIG   = "config";    ///< reload configuration, apply what can be applied
const std::string ACAMD_COVER    = "cover";     ///< cover [ open|close ] to set or get the dust over
const std::string ACAMD_ECHO     = "echo";      ///< 
const std::string ACAMD_GAIN     = "gain";      ///< set/get CCD Gain
const std::string ACAMD_IMFLIP   = "imflip";    ///< set/get CCD image flip states
const std::string ACAMD_IMROT    = "imrot";     ///< set/get CCD image rotation states
const std::string ACAMD_SPEED    = "speed";     ///< set/get CCD clocking speeds
const std::string ACAMD_EXIT     = "exit";      ///< 
const std::string ACAMD_EXPTIME  = "exptime";   ///< set/get camera exposure time
const std::string ACAMD_FILTER   = "filter";    ///< filter [ name ] to set or get the filter
const std::string ACAMD_HOME     = "home";      ///< home all motors
const std::string ACAMD_INIT     = "init";      ///< ***
const std::string ACAMD_ISHOME   = "ishome";    ///< are all motors homed?
const std::string ACAMD_ISOPEN   = "isopen";    ///< *** close connection to all devices
const std::string ACAMD_MOTION   = "motion";    ///< motion commands primarily for CLI testing
const std::string ACAMD_OPEN     = "open";      ///< *** open connection to all devices, camera and motion
const std::string ACAMD_QUALITY  = "quality";   ///< *** call the Python telemetry function
const std::string ACAMD_SOLVE    = "solve";     ///< *** call the Python astrometry solver
const std::string ACAMD_TEMP     = "temp";      ///< set/get acam temperature
const std::vector<std::string> ACAMD_SYNTAX = { 
                                                "  SERVER COMMANDS:",
                                                ACAMD_CLOSE,
                                                ACAMD_CONFIG+" [ ? ]",
                                                ACAMD_ECHO+" ? | <string>",
                                                ACAMD_EXIT,
                                                ACAMD_ISOPEN+" [ ? | motion | camera ]",
                                                ACAMD_OPEN+" [ ? | [motion] [camera [<args>]] ]",
                                                "  MOTION COMMANDS:",
                                                ACAMD_COVER+" [ ? | open | close | home | ishome ]",
                                                ACAMD_FILTER+" [ ? | <filtername> | home | ishome ]",
                                                ACAMD_HOME+" [ ? | <name> ]",
                                                ACAMD_ISHOME+" [ ? | <name> ]",
                                                ACAMD_MOTION+" [ ? | [ <name> [ native <cmd> | <posname> ] ] ]",
                                                "  CAMERA COMMANDS:",
                                                ACAMD_ACQUIRE+" [ ? ]",
                                                ACAMD_BIN+" [ ? | <hbin> <vbin> ]",
                                                ACAMD_EXPTIME+" [ ? | <exptime> ]",
                                                ACAMD_GAIN+" [ ? | <gain> ]",
                                                ACAMD_IMFLIP+" [ ? | <hflip> <vflip> ]",
                                                ACAMD_IMROT+" [ ? | <rotdir> ]",
                                                ACAMD_INIT,
#ifdef ACAM_ANDOR_SOURCE_SERVER
                                                ACAMD_CAMERASERVER_COORDS+" <ra> <dec> <pa>",
#endif
                                                ACAMD_QUALITY+" [ ? ]",
                                                ACAMD_SOLVE+" [ ? | <filename>] [ <key>=<val> ... ]",
                                                ACAMD_SPEED+" [ ? | <hori> <vert> ]",
                                                ACAMD_TEMP+" [ ? | <setpoint> ]"
                                              };
#endif
