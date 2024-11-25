/**
 * @file    calibd_commands.h
 * @brief   defines the commands accepted by the calib daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#pragma once

const std::string CALIBD_CLOSE = "close";    ///< close connection to calibd hardware
const std::string CALIBD_EXIT = "exit";      ///< is calib open
const std::string CALIBD_GET = "get";        ///< get state of both actuators
const std::string CALIBD_HOME = "home";      const int CALIBD_HOME_TIMEOUT = 60000; ///< home all actuators
const std::string CALIBD_ISHOME = "ishome";  ///< are calib actuators homed?
const std::string CALIBD_ISOPEN = "isopen";  ///< is calib open
const std::string CALIBD_LAMPMOD = "lampmod";///< lamp modulator control
const std::string CALIBD_NATIVE = "native";  ///< send native commands to PI controller
const std::string CALIBD_OPEN = "open";      ///< open connection to calib
const std::string CALIBD_SET = "set";        const int CALIBD_SET_TIMEOUT = 25000; ///< set state of both actuators

const std::vector<std::string> CALIBD_SYNTAX = {
                                                 CALIBD_CLOSE,
                                                 CALIBD_EXIT,
                                                 "  MOTION CONTROL",
                                                 CALIBD_GET+" [ <actuator> ] | [?]",
                                                 CALIBD_HOME+" [?]",
                                                 CALIBD_ISHOME,
                                                 CALIBD_ISOPEN,
                                                 CALIBD_NATIVE+" <addr> <cmd>",
                                                 CALIBD_OPEN,
                                                 CALIBD_SET+" [ <actuator>=open|close ... ] | [?]",
                                                 "  LAMP MODULATOR CONTROL",
                                                 CALIBD_LAMPMOD+" ? | open | close | reconnect | default | <n> [ [ on|off ] | [ <D> <T> ] ]",
                                                 "  OTHER",
                                                 TELEMREQUEST+" [ ? ]"
                                               };
