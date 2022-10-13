/**
 * @file    sequencerd_commands.h
 * @brief   defines the commands accepted by the sequencer daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef SEQEUNCERD_COMMANDS_H
#define SEQEUNCERD_COMMANDS_H
const std::string SEQUENCERD_STARTUP = "startup";
const std::string SEQUENCERD_START   = "start";
const std::string SEQUENCERD_STOP    = "stop";
const std::string SEQUENCERD_ABORT   = "abort";
const std::string SEQUENCERD_PAUSE   = "pause";
const std::string SEQUENCERD_RESUME  = "resume";
const std::string SEQUENCERD_NEXT    = "next";
const std::string SEQUENCERD_EXIT    = "exit";
const std::string SEQUENCERD_TEST    = "test";

const std::string SEQUENCERD_CALIB   = "calib";
const std::string SEQUENCERD_CAMERA  = "camera";
const std::string SEQUENCERD_FILTER  = "filter";
const std::string SEQUENCERD_POWER   = "power";
const std::string SEQUENCERD_SLIT    = "slit";
const std::string SEQUENCERD_TCS     = "tcs";
#endif
