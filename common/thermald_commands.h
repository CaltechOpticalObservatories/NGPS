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
const std::string THERMALD_NATIVE   = "native";    ///< 
const std::string THERMALD_RECONNECT = "reconnect";   ///< 
const std::string THERMALD_SETPOINT = "setpoint";  ///< 
const std::string THERMALD_SHOWTELEM = "show";  ///< 
const std::string THERMALD_TELEMETRY = "telem";  ///< 
const std::vector<std::string> THERMALD_SYNTAX = { 
                                                 THERMALD_ECHO+" ? | <string>",
                                                 THERMALD_EXIT,
                                                 THERMALD_GET+" [ ? | <label> | <unt> <chan> ] | camp [force]",
                                                 THERMALD_NATIVE+" [ ? | <unit> <cmd> [<args>] ]",
                                                 THERMALD_RECONNECT+" [ ? ]",
                                                 TELEMREQUEST+" [?]",
                                                 THERMALD_SETPOINT+" [ ? | <unit> <output> [ <temp> ] ]",
                                                 THERMALD_SHOWTELEM+" [ ? | force ]",
                                                 THERMALD_TELEMETRY+" ? | start | stop | status "
                                                 };
#endif
