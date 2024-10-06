/**
 * @file    focusd_commands.h
 * @brief   defines the commands accepted by the focus daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#pragma once

const std::string FOCUSD_CLOSE = "close";    const int FOCUSD_CLOSE_TIMEOUT = 3000;
const std::string FOCUSD_GET = "get";        const int FOCUSD_GET_TIMEOUT = 3000;
const std::string FOCUSD_HOME = "home";      const int FOCUSD_HOME_TIMEOUT = 15000;
const std::string FOCUSD_ISHOME = "ishome";  const int FOCUSD_ISHOME_TIMEOUT = 3000;
const std::string FOCUSD_ISOPEN = "isopen";  const int FOCUSD_ISOPEN_TIMEOUT = 3000;
const std::string FOCUSD_NATIVE = "native";  const int FOCUSD_NATIVE_TIMEOUT = 3000;
const std::string FOCUSD_OPEN = "open";      const int FOCUSD_OPEN_TIMEOUT = 3000;
const std::string FOCUSD_SET  = "set";       const int FOCUSD_SET_TIMEOUT = 15000;
const std::string FOCUSD_TELEMREQUEST="telem";
const std::string FOCUSD_TEST  = "test";     const int FOCUSD_TEST_TIMEOUT = 15000;

const std::vector<std::string> FOCUSD_SYNTAX = {
                                                 FOCUSD_CLOSE,
                                                 FOCUSD_GET+" ? | <chan>",
                                                 FOCUSD_HOME+" [ ? | <chan> ]",
                                                 FOCUSD_ISHOME+" [ ? ]",
                                                 FOCUSD_ISOPEN,
                                                 FOCUSD_NATIVE+" ? | <chan> <cmd>",
                                                 FOCUSD_OPEN,
                                                 FOCUSD_SET+" ? | <chan> { <pos> | nominal }",
                                                 FOCUSD_TELEMREQUEST+" [ ? ]",
                                                 FOCUSD_TEST+" <testname> ...",
                                                 "   motormap",
                                                 "   posmap",
                                               };

