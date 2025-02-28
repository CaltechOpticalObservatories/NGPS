/**
 * @file    acamd_commands.h
 * @brief   the Acam::Server::doit() function listens for these commands.
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 *
 */

#pragma once

const std::string ACAMD_ACQUIRE  = "acquire";   ///< acquire target
const std::string ACAMD_AVGFRAMES= "avgframes"; ///< average frames
const std::string ACAMD_BIN      = "bin";       ///< set/get camera binning
const std::string ACAMD_CLOSE    = "close";     ///< *** close connection to all devices
const std::string ACAMD_CONFIG   = "config";    ///< reload configuration, apply what can be applied
const std::string ACAMD_COORDS   = "coords";    ///< set/get target coords for acquire
const std::string ACAMD_COVER    = "cover";     ///< cover [ open|close ] to set or get the dust over
const std::string ACAMD_ECHO     = "echo";      ///< 
const std::string ACAMD_FRAMEGRAB  = "framegrab";   ///< *** the main acquire wrapper
const std::string ACAMD_FRAMEGRABFIX= "framegrabfix";   ///< acquire using the WCSfix file from solve
const std::string ACAMD_GAIN     = "gain";      ///< set/get CCD Gain
const std::string ACAMD_IMFLIP   = "imflip";    ///< set/get CCD image flip states
const std::string ACAMD_IMROT    = "imrot";     ///< set/get CCD image rotation states
const std::string ACAMD_SPEED    = "speed";     ///< set/get CCD clocking speeds
const std::string ACAMD_EMULATOR = "emulator";  ///< set/get Andor emulator state
const std::string ACAMD_EXIT     = "exit";      ///< 
const std::string ACAMD_EXPTIME  = "exptime";   ///< set/get camera exposure time
const std::string ACAMD_FAN      = "fan";       ///< set Andor fan mode
const std::string ACAMD_FILTER   = "filter";    ///< filter [ name ] to set or get the filter
const std::string ACAMD_GUIDESET = "guideset";  ///< set params for guider display
const std::string ACAMD_HOME     = "home";      const int ACAMD_HOME_TIMEOUT = 180000; ///< home all motors
                                                const int ACAMD_MOVE_TIMEOUT = 40000;  ///< covers filter and cover moves
const std::string ACAMD_INIT     = "init";      ///< ***
const std::string ACAMD_ISACQUIRED = "isacquired";  ///< is the target acquired?
const std::string ACAMD_ISHOME   = "ishome";    ///< are all motors homed?
const std::string ACAMD_ISOPEN   = "isopen";    ///< *** close connection to all devices
const std::string ACAMD_LOADCALIBRATION = "loadfpcal";    ///< 
const std::string ACAMD_MOTION   = "motion";    ///< motion commands primarily for CLI testing
const std::string ACAMD_OFFSETCAL = "offsetcal";  ///< perform TCS offset calibration
const std::string ACAMD_OFFSETGOAL = "offsetgoal";  ///< add dRA,dDEC offset to goal
const std::string ACAMD_OFFSETPERIOD = "offsetperiod";  ///< period to send guide offsets to TCS
const std::string ACAMD_OPEN     = "open";      const int ACAMD_OPEN_TIMEOUT = 180000; ///< *** open connection to all devices, camera and motion
const std::string ACAMD_PUTONSLIT= "putonslit"; ///< put target on slit
const std::string ACAMD_QUALITY  = "quality";   ///< *** call the Python telemetry function
const std::string ACAMD_SHUTDOWN = "shutdown";  const int ACAMD_SHUTDOWN_TIMEOUT = 40000; ///< shutdown threads and close connections
const std::string ACAMD_SAVEFRAMES= "saveframes";     ///< num frames to save during target acquire
const std::string ACAMD_SKIPFRAMES= "skipframes";     ///< num frames to skip before target acquire
const std::string ACAMD_SOLVE    = "solve";     ///< *** call the Python astrometry solver
const std::string ACAMD_TCSGET = "tcsget";      ///< 
const std::string ACAMD_TCSINIT = "tcsinit";    ///< initialize acamd's connection to tcsd
const std::string ACAMD_TCSISCONNECTED = "tcsisconnected";  ///< 
const std::string ACAMD_TCSISOPEN = "tcsopen";  ///<
const std::string ACAMD_TEMP     = "temp";      ///< set/get acam temperature
const std::string ACAMD_TEST     = "test";      ///< test commands
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
                                                "  TCS COMMANDS:",
                                                ACAMD_TCSGET+" [ ? ]",
                                                ACAMD_TCSINIT+" [ ? | tcs | sim ]",
                                                ACAMD_TCSISCONNECTED+" [ ? ]",
                                                ACAMD_TCSISOPEN+" [ ? ]",
                                                "  CAMERA COMMANDS:",
                                                ACAMD_AVGFRAMES+" [ ? | reset | <num> ]",
                                                ACAMD_FRAMEGRAB+" [ ? | start | stop | one [ <filename> ] | saveone <filename> | status ]",
                                                ACAMD_FRAMEGRABFIX+" [ ? ]",
                                                ACAMD_BIN+" [ ? | <hbin> <vbin> ]",
                                                ACAMD_EMULATOR+" [ ? | true | false ]",
                                                ACAMD_EXPTIME+" [ ? | <exptime> ]",
                                                ACAMD_FAN+" [ ? | <mode> ]",
                                                ACAMD_GAIN+" [ ? | <gain> ]",
                                                ACAMD_GUIDESET+" [ ? | <exptime> <gain> <filter> <focus> ]",
                                                ACAMD_IMFLIP+" [ ? | <hflip> <vflip> ]",
                                                ACAMD_IMROT+" [ ? | <rotdir> ]",
                                                ACAMD_INIT,
                                                ACAMD_QUALITY+" [ ? ]",
                                                ACAMD_SAVEFRAMES+" [ ? | <nsave> ]",
                                                ACAMD_SKIPFRAMES+" [ ? | <nskip> ]",
                                                ACAMD_SOLVE+" [ ? | <filename>] [ <key>=<val> ... ]",
                                                ACAMD_SPEED+" [ ? | <hori> <vert> ]",
                                                ACAMD_TEMP+" [ ? | <setpoint> ]",
                                                "  OTHER:",
                                                ACAMD_ACQUIRE+" [ ? | [ <ra> <dec> <angle> | target | guide | stop ] ]",
                                                ACAMD_COORDS+" [ ? | [ <ra> <dec> <angle> ] ]",
                                                ACAMD_ISACQUIRED,
                                                ACAMD_LOADCALIBRATION+"",
                                                ACAMD_OFFSETCAL+" [ ? ]",
                                                ACAMD_OFFSETGOAL+" [ ? | <dRA> <dDEC> ]",
                                                ACAMD_OFFSETPERIOD+" [ ? | <sec> ]",
                                                ACAMD_PUTONSLIT+" [ ? | <crossra> <crossdec> ]",
                                                TELEMREQUEST+" [ ? ]",
                                                ACAMD_SHUTDOWN+" [ ? ]",
                                                ACAMD_TEST+" ? | <testname> ..."
                                              };
