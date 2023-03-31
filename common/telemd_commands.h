/**
 * @file    telemd_commands.h
 * @brief   the Telemetry::Server::doit() function listens for these commands.
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 *
 */

#ifndef TELEMD_COMMANDS_H
#define TELEMD_COMMANDS_H
const std::string TELEMD_EXIT     = "exit";      ///< 
const std::string TELEMD_ECHO     = "echo";      ///< 
const std::vector<std::string> TELEMD_SYNTAX = { 
                                                 TELEMD_EXIT,
                                                 TELEMD_ECHO
                                              };
#endif
