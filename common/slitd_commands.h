/**
 * @file    slitd_commands.h
 * @brief   defines the commands accepted by the slit daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef SLITD_COMMANDS_H
#define SLITD_COMMANDS_H
const std::string SLITD_CLOSE = "close";    const int SLITD_CLOSE_TIMEOUT = 3000;
const std::string SLITD_GET = "get";        const int SLITD_GET_TIMEOUT = 3000;
const std::string SLITD_HOME = "home";      const int SLITD_HOME_TIMEOUT = 15000;
const std::string SLITD_ISHOME = "ishome";  const int SLITD_ISHOME_TIMEOUT = 3000;
const std::string SLITD_ISOPEN = "isopen";  const int SLITD_ISOPEN_TIMEOUT = 3000;
const std::string SLITD_OPEN = "open";      const int SLITD_OPEN_TIMEOUT = 3000;
const std::string SLITD_SET  = "set";       const int SLITD_SET_TIMEOUT = 15000;
#endif
