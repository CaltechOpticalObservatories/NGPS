/**
 * @file    camerad_commands.h
 * @brief   defines the commands accepted by the camera daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef CAMERAD_COMMANDS_H
#define CAMERAD_COMMANDS_H
const std::string CAMERAD_OPEN = "open";
const std::string CAMERAD_ISOPEN = "isopen";
const std::string CAMERAD_EXPTIME = "exptime";
const std::string CAMERAD_EXPOSE = "expose";
const std::string CAMERAD_ABORT = "abort";
const std::string CAMERAD_PAUSE = "pause";
const std::string CAMERAD_RESUME = "resume";
const std::string CAMERAD_STOP = "stop";
const std::string CAMERAD_MODEXPTIME = "modexptime";
#endif
