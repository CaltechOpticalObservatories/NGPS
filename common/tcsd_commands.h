/**
 * @file    tcsd_commands.h
 * @brief   defines the commands accepted by the tcs daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef TCSD_COMMANDS_H
#define TCSD_COMMANDS_H
const std::string TCSD_CLOSE = "close";
const std::string TCSD_COORDS = "coords";
const std::string TCSD_EXIT = "exit";
const std::string TCSD_GET_CASS = "getcass";
const std::string TCSD_GET_COORDS = "getcoords";
const std::string TCSD_GET_DOME = "getdome";
const std::string TCSD_GET_FOCUS = "getfocus";
const std::string TCSD_GET_MOTION = "getmotion";
const std::string TCSD_GET_OFFSETS = "getoffsets";
const std::string TCSD_GET_NAME = "getname";
const std::string TCSD_ISOPEN = "isopen";
const std::string TCSD_LIST = "list";
const std::string TCSD_LLIST = "llist";
const std::string TCSD_NATIVE = "native";
const std::string TCSD_OFFSETRATE = "offsetrate";
const std::string TCSD_OPEN = "open";
const std::string TCSD_PTOFFSET = "offset";
const std::string TCSD_RETOFFSETS = "retoffsets";
const std::string TCSD_RINGGO = "ringgo";
const std::string TCSD_SET_FOCUS = "setfocus";
const std::string TCSD_TELEMREQUEST = "telem";
const std::string TCSD_WEATHER_COORDS = "weathercoords";

const std::vector<std::string> TCSD_SYNTAX = {
                                               TCSD_CLOSE,
                                               TCSD_COORDS+" ? | <ra> <dec> <equinox> <ramotion> <decmotion> [<motionflag>] [ \"<targetname>\" ]",
                                               TCSD_EXIT,
                                               TCSD_GET_CASS+" [ ? ]",
                                               TCSD_GET_COORDS+" [ ? ]",
                                               TCSD_GET_DOME+" [ ? ]",
                                               TCSD_GET_FOCUS+" [ ? ]",
                                               TCSD_GET_MOTION+" [ ? ]",
                                               TCSD_GET_OFFSETS+" [ ? ]",
                                               TCSD_GET_NAME+" [ ? ]",
                                               TCSD_ISOPEN+" [ ? ]",
                                               TCSD_LIST+" [ ? ]",
                                               TCSD_LLIST+" [ ? ]",
                                               TCSD_NATIVE+" ? | <cmd>",
                                               TCSD_OFFSETRATE+" [ ? | <raoff> <decoff> ]",
                                               TCSD_OPEN+" ? | <name>",
                                               TCSD_PTOFFSET+" ? | <ra> <dec>",
                                               TCSD_RETOFFSETS+" [ ? ]",
                                               TCSD_RINGGO+" ? | <angle>",
                                               TCSD_SET_FOCUS+" ? | <value>",
                                               TCSD_TELEMREQUEST+" [ ? ]",
                                               TCSD_WEATHER_COORDS+" [ ? ]"
                                             };
#endif
