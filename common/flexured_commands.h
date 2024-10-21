/**
 * @file    flexured_commands.h
 * @brief   defines the commands accepted by the flexure daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#pragma once

const std::string FLEXURED_CLOSE = "close";    const int FLEXURED_CLOSE_TIMEOUT = 3000;
const std::string FLEXURED_GET = "get";        const int FLEXURED_GET_TIMEOUT = 3000;
const std::string FLEXURED_ISOPEN = "isopen";  const int FLEXURED_ISOPEN_TIMEOUT = 3000;
const std::string FLEXURED_NATIVE = "native";  const int FLEXURED_NATIVE_TIMEOUT = 3000;
const std::string FLEXURED_OPEN = "open";      const int FLEXURED_OPEN_TIMEOUT = 3000;
const std::string FLEXURED_SET  = "set";       const int FLEXURED_SET_TIMEOUT = 15000;
const std::string FLEXURED_TEST  = "test";     const int FLEXURED_TEST_TIMEOUT = 15000;

const std::vector<std::string> FLEXURED_SYNTAX = {
                                                 FLEXURED_CLOSE,
                                                 FLEXURED_GET+" ? | <chan> <axis>",
                                                 FLEXURED_ISOPEN,
                                                 FLEXURED_NATIVE+" ? | <chan> <cmd>",
                                                 FLEXURED_OPEN,
                                                 FLEXURED_SET+" ? | <chan> <axis> <pos>",
                                                 TELEMREQUEST+" [ ? ]",
                                                 FLEXURED_TEST+" ? | <testname> ...",
                                                 "   motormap",
                                                 "   posmap",
                                               };
