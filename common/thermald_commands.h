/**
 * @file    thermald_commands.h
 * @brief   the Telemetry::Server::doit() function listens for these commands.
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 *
 */

#ifndef THERMALD_COMMANDS_H
#define THERMALD_COMMANDS_H
const std::string THERMALD_ECHO     = "echo";      ///< 
const std::string THERMALD_EXIT     = "exit";      ///< 
const std::string THERMALD_GET      = "get";       ///< 
const std::string THERMALD_LOGALL   = "logall";    ///< 
const std::string THERMALD_NATIVE   = "native";    ///< 
const std::string THERMALD_READALL  = "readall";   ///< 
const std::string THERMALD_SETPOINT = "setpoint";  ///< 
const std::vector<std::string> THERMALD_SYNTAX = { 
                                                 THERMALD_ECHO+" ? | <string>",
                                                 THERMALD_EXIT,
                                                 THERMALD_GET+" [ ? | <label> | <unt> <chan> ]",
                                                 THERMALD_LOGALL+" [ ? ]",
                                                 THERMALD_NATIVE+" [ ? | <unit> <cmd> [<args>] ]",
                                                 THERMALD_READALL+" [ ? ]",
                                                 THERMALD_SETPOINT+" [ ? | <unit> <output> [ <temp> ] ]"
                                                 };
#endif
