/**
 * @file    flexured_commands.h
 * @brief   defines the commands accepted by the flexure daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef FLEXURED_COMMANDS_H
#define FLEXURED_COMMANDS_H

const std::string FLEXURED_CLOSE = "close";    const int FLEXURED_CLOSE_TIMEOUT = 3000;
const std::string FLEXURED_GET = "get";        const int FLEXURED_GET_TIMEOUT = 3000;
const std::string FLEXURED_HOME = "home";      const int FLEXURED_HOME_TIMEOUT = 15000;
const std::string FLEXURED_ISHOME = "ishome";  const int FLEXURED_ISHOME_TIMEOUT = 3000;
const std::string FLEXURED_ISOPEN = "isopen";  const int FLEXURED_ISOPEN_TIMEOUT = 3000;
const std::string FLEXURED_NATIVE = "native";  const int FLEXURED_NATIVE_TIMEOUT = 3000;
const std::string FLEXURED_OPEN = "open";      const int FLEXURED_OPEN_TIMEOUT = 3000;
const std::string FLEXURED_SET  = "set";       const int FLEXURED_SET_TIMEOUT = 15000;

const std::vector<std::string> FLEXURED_SYNTAX = {
                                                 FLEXURED_CLOSE,
                                                 FLEXURED_GET,
                                                 FLEXURED_HOME,
                                                 FLEXURED_ISHOME,
                                                 FLEXURED_ISOPEN,
                                                 FLEXURED_NATIVE,
                                                 FLEXURED_OPEN,
                                                 FLEXURED_SET
                                               };

#endif
