/**
 * @file    common_commands.h
 * @brief   command tokens common to all NGPS daemons
 * @details These are command strings accepted by every daemon's command
 *          interface and used by external tools (e.g. the hang watchdog,
 *          ngps_watchdog). Defining them once here and including this from each
 *          <daemon>_commands.h gives a single source of truth for the wire token,
 *          so the daemon side and the watchdog side cannot drift.
 * @author  NGPS
 *
 */

#ifndef COMMON_COMMANDS_H
#define COMMON_COMMANDS_H

#include <string>

const std::string CMD_PING = "ping";   ///< liveness probe (external watchdog); reply is CMD_PONG
const std::string CMD_PONG = "pong";   ///< liveness reply to CMD_PING

#endif
