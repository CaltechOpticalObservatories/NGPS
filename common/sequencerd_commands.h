/**
 * @file    sequencerd_commands.h
 * @brief   defines the commands accepted by the sequencer daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef SEQEUNCERD_COMMANDS_H
#define SEQEUNCERD_COMMANDS_H
const std::string SEQUENCERD_ABORT      = "abort";
const std::string SEQUENCERD_DOTYPE     = "do";
const std::string SEQUENCERD_DOMENOWAIT = "domenowait";
const std::string SEQUENCERD_EXIT       = "exit";
const std::string SEQUENCERD_MODEXPTIME = "modexptime";
const std::string SEQUENCERD_PAUSE      = "pause";
const std::string SEQUENCERD_RESUME     = "resume";
const std::string SEQUENCERD_SHUTDOWN   = "shutdown";
const std::string SEQUENCERD_START      = "start";
const std::string SEQUENCERD_STARTUP    = "startup";
const std::string SEQUENCERD_STATE      = "state";
const std::string SEQUENCERD_STOP       = "stop";
const std::string SEQUENCERD_TARGETSET  = "targetset";
const std::string SEQUENCERD_TCSNOWAIT  = "tcsnowait";
const std::string SEQUENCERD_TEST       = "test";

const std::string SEQUENCERD_ACAM       = "acam";
const std::string SEQUENCERD_CALIB      = "calib";
const std::string SEQUENCERD_CAMERA     = "camera";
const std::string SEQUENCERD_FILTER     = "filter";
const std::string SEQUENCERD_POWER      = "power";
const std::string SEQUENCERD_SLIT       = "slit";
const std::string SEQUENCERD_TCS        = "tcs";
const std::vector<std::string> SEQUENCERD_SYNTAX = {
                                                     SEQUENCERD_ACAM+" ...",
                                                     SEQUENCERD_CALIB+" ...",
                                                     SEQUENCERD_CAMERA+" ...",
                                                     SEQUENCERD_FILTER+" ...",
                                                     SEQUENCERD_POWER+" ...",
                                                     SEQUENCERD_SLIT+" ...",
                                                     SEQUENCERD_TCS+" ...",
                                                     "",
                                                     SEQUENCERD_ABORT,
                                                     SEQUENCERD_DOMENOWAIT,
                                                     SEQUENCERD_DOTYPE+" [ one | all ]",
                                                     SEQUENCERD_EXIT,
                                                     SEQUENCERD_MODEXPTIME+" <exptime>",
                                                     SEQUENCERD_PAUSE,
                                                     SEQUENCERD_RESUME,
                                                     SEQUENCERD_SHUTDOWN,
                                                     SEQUENCERD_START,
                                                     SEQUENCERD_STARTUP,
                                                     SEQUENCERD_STATE,
                                                     SEQUENCERD_STOP,
                                                     SEQUENCERD_TARGETSET+" [ <targetid> | <targetname> ]",
                                                     SEQUENCERD_TCSNOWAIT,
                                                     SEQUENCERD_TEST+" <testname> ...",
                                                     "   addrow <num> <name> <ra> <dec>",
                                                     "   async [ <message> ]",
                                                     "   completed",
                                                     "   fpoffset <from> <to>",
                                                     "   getnext",
                                                     "   moveto",
                                                     "   notify",
                                                     "   pause",
                                                     "   radec",
                                                     "   resume",
                                                     "   states",
                                                     "   tablenames",
                                                     "   update { pending | complete | unassigned }",
                                                   };
#endif
