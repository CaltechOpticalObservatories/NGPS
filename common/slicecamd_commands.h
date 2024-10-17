/**
 * @file    slicecamd_commands.h
 * @brief   the Acam::Server::doit() function listens for these commands.
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 *
 */

#pragma once

const std::string SLICECAMD_BIN      = "bin";       ///< set/get camera binning
const std::string SLICECAMD_CLOSE    = "close";     ///< *** close connection to all devices
const std::string SLICECAMD_CONFIG   = "config";    ///< reload configuration, apply what can be applied
const std::string SLICECAMD_ECHO     = "echo";      ///< 
const std::string SLICECAMD_FRAMEGRAB  = "framegrab";   ///< *** the main acquire wrapper
const std::string SLICECAMD_FRAMEGRABFIX= "framegrabfix";   ///< acquire using the WCSfix file from solve
const std::string SLICECAMD_GAIN     = "gain";      ///< set/get CCD Gain
const std::string SLICECAMD_IMFLIP   = "imflip";    ///< set/get CCD image flip states
const std::string SLICECAMD_IMROT    = "imrot";     ///< set/get CCD image rotation states
const std::string SLICECAMD_SPEED    = "speed";     ///< set/get CCD clocking speeds
const std::string SLICECAMD_EMULATOR = "emulator";  ///< set/get Andor emulator state
const std::string SLICECAMD_EXIT     = "exit";      ///< 
const std::string SLICECAMD_EXPTIME  = "exptime";   ///< set/get camera exposure time
const std::string SLICECAMD_GUISET   = "guiset";    ///< set params for gui display
const std::string SLICECAMD_INIT     = "init";      ///< ***
const std::string SLICECAMD_ISACQUIRED = "isacquired";  ///< is the target acquired?
const std::string SLICECAMD_ISOPEN   = "isopen";    ///< *** close connection to all devices
const std::string SLICECAMD_OPEN     = "open";      ///< *** open connection to all devices, camera and motion
const std::string SLICECAMD_PUTONSLIT= "putonslit"; ///< put target on slit
const std::string SLICECAMD_TELEMREQUEST= "sendtelem"; ///< send my telemetry as JSON message
const std::string SLICECAMD_TCSGET = "tcsget";      ///< 
const std::string SLICECAMD_TCSINIT = "tcsinit";    ///< initialize slicecamd's connection to tcsd
const std::string SLICECAMD_TCSISCONNECTED = "tcsisconnected";  ///< 
const std::string SLICECAMD_TCSISOPEN = "tcsopen";  ///<
const std::string SLICECAMD_TEMP     = "temp";      ///< set/get slicecam temperature
const std::string SLICECAMD_TEST     = "test";      ///< test commands
const std::vector<std::string> SLICECAMD_SYNTAX = { 
                                                "  SERVER COMMANDS:",
                                                SLICECAMD_CLOSE,
                                                SLICECAMD_CONFIG+" [ ? ]",
                                                SLICECAMD_ECHO+" ? | <string>",
                                                SLICECAMD_EXIT,
                                                SLICECAMD_ISOPEN+" [ ? | motion | camera ]",
                                                SLICECAMD_OPEN+" [ ? | [motion] [camera [<args>]] ]",
                                                "  TCS COMMANDS:",
                                                SLICECAMD_TCSGET+" [ ? ]",
                                                SLICECAMD_TCSINIT+" [ ? | tcs | sim ]",
                                                SLICECAMD_TCSISCONNECTED+" [ ? ]",
                                                SLICECAMD_TCSISOPEN+" [ ? ]",
                                                "  CAMERA COMMANDS:",
                                                SLICECAMD_FRAMEGRAB+" [ ? | start | stop | one [ <filename> ] | status ]",
                                                SLICECAMD_FRAMEGRABFIX+" [ ? ]",
                                                SLICECAMD_BIN+" [ ? | <hbin> <vbin> ]",
                                                SLICECAMD_EMULATOR+" [ ? | true | false ]",
                                                SLICECAMD_EXPTIME+" [ ? | <exptime> ]",
                                                SLICECAMD_GAIN+" [ ? | <gain> ]",
                                                SLICECAMD_GUISET+" [ ? | <exptime> <gain> ]",
                                                SLICECAMD_IMFLIP+" [ ? | <hflip> <vflip> ]",
                                                SLICECAMD_IMROT+" [ ? | <rotdir> ]",
                                                SLICECAMD_INIT,
                                                SLICECAMD_SPEED+" [ ? | <hori> <vert> ]",
                                                SLICECAMD_TEMP+" [ ? | <setpoint> ]",
                                                "  OTHER:",
                                                SLICECAMD_PUTONSLIT+" [ ? | <slitra> <slitdec> <crossra> <crossdec> ]",
                                                SLICECAMD_TELEMREQUEST+" [ ? ]",
                                                SLICECAMD_TEST+" ? | <testname> ..."
                                              };
