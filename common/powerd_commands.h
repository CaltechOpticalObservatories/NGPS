/**
 * @file    powerd_commands.h
 * @brief   defines the commands accepted by the power daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#pragma once

const std::string POWERD_CLOSE  = "close";
const std::string POWERD_ISOPEN = "isopen";
const std::string POWERD_LIST   = "list";
const std::string POWERD_OPEN   = "open";
const std::string POWERD_REOPEN = "reopen";
const std::string POWERD_STATUS = "status";
const std::string POWERD_TELEMREQUEST = "telem";
const std::vector<std::string> POWERD_SYNTAX = {
                                                 POWERD_CLOSE,
                                                 POWERD_ISOPEN,
                                                 POWERD_LIST+" [?]",
                                                 POWERD_OPEN,
                                                 POWERD_REOPEN+" [?]",
                                                 POWERD_STATUS+" [?]",
                                                 POWERD_TELEMREQUEST+" [?]",
                                                 "",
                                                 "<unit#> <plug#> [ ON | OFF | BOOT ]",
                                                 "<plugname> [ ON | OFF | BOOT ]"
                                               };
