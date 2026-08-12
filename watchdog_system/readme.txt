# @file     readme.txt
# @brief    readme file for ngps watchdog system
# @details  how to set up the watchdog
# @author   David Hale
#

==============================================================================
NGPS daemon system control + watchdog
==============================================================================

------------------------------------------------------------------------------
Quick start
------------------------------------------------------------------------------
Prerequisites: accounts "dataowner"/"datawriters" (daemons), "developer"
(builds + owns the ngps wrapper), and any interactive operators (e.g.
"observer") must already exist on the host. The watchdog branch must be
built (BASEDIR/bin/ngps-watchdog present and executable).

Install (as root or via sudo, from the watchdog_system/ directory):

    sudo ./install-watchdog

Uninstall -- disaster-recovery/rollback only; returns the host to the
legacy `_ngps`/`ngps-daemon` boot path (see "What install-watchdog /
uninstall-watchdog do" below):

    sudo ./uninstall-watchdog

Both accept the same overridable path variables if the deploy layout
differs from the default (WATCHDOG_DIR, BIN_DIR, RUN_DIR) -- see the
comments at the top of each script. That's the whole procedure; everything
below explains what the two scripts do and how to operate the daemons
afterward.


------------------------------------------------------------------------------
How it works -- three safety nets
------------------------------------------------------------------------------
  Layer 1  systemd restarts any daemon that EXITS (crash/signal/exit 0).
  Layer 2  ngps-watchdog restarts any daemon that is alive but FROZEN.
  Layer 3  systemd watches ngps-watchdog itself (WatchdogSec heartbeat).
Plus: the sequencer restarts a peer it cannot talk to, via systemctl.


------------------------------------------------------------------------------
Requirements (must already be true before running install-watchdog)
------------------------------------------------------------------------------
BASEDIR : /home/developer/Software  (binaries in BASEDIR/bin, configs in BASEDIR/Config).

Service account / group: user "dataowner", group "datawriters".
  - daemons run as this user/group
  - the sequencer runs as dataowner and restarts peers via systemctl

Developer account: user "developer".
  - owns and may execute watchdog_system/bin/ngps (see "the ngps wrapper"
    below) -- install-watchdog installs a restricted copy for this
    account only, at run/ngps
  - must also be a member of "ngpsops" (install-watchdog adds it
    automatically, see below) -- without that membership, the wrapper's
    `systemctl` calls would hit a password prompt instead of the
    passwordless path the polkit rule provides

Operator group: "ngpsops".
  - members may run `systemctl {start|stop|restart} ngps.target` and
    `systemctl ... ngps@<name>.service` without a password (per polkit
    rule, installed by install-watchdog). They cannot control
    ngps-watchdog.service -- that one requires admin auth, by design.
  - datawriters is for write access into the data tree and must NOT be
    used to grant daemon-control rights -- keep the two groups distinct.
  - install-watchdog creates this group if missing and adds "developer"
    and "observer" to it automatically (both must already exist as
    accounts). Add any additional interactive operators yourself:
        sudo usermod -aG ngpsops <username>
    Operators must log out and back in for new group membership to take
    effect.


------------------------------------------------------------------------------
What install-watchdog / uninstall-watchdog do
------------------------------------------------------------------------------
install-watchdog:
  - Disables the legacy ngps-daemon.timer/.service so its Restart=
    behavior does not fight the new units, and removes any live
    run/ngps, run/_ngps, /usr/local/bin/ngps leftover (normally a
    no-op -- see below).
  - Installs the systemd units (ngps@.service, ngps.target,
    ngps-watchdog.service, ngps@sequencerd.service.d/order.conf) to
    /etc/systemd/system/.
  - Installs /etc/sysconfig/ngps from systemd/ngps.env, only if it
    doesn't already exist (preserves operator edits on reinstall).
  - Installs the polkit rule (10-ngps.rules) and restarts polkit.
  - Creates the "ngpsops" group if missing, and adds "developer" and
    "observer" to it.
  - Reloads systemd and enables+starts ngps.target and
    ngps-watchdog.service.
  - Locks down watchdog_system/bin/ngps to owner developer, mode 744,
    and installs a mode-700 copy at $RUN_DIR/ngps (default
    .../Software/run/ngps) -- see "the ngps wrapper" below.

uninstall-watchdog reverses the above in the opposite order: stops and
disables the ngps units, removes the unit files/env file/polkit rule,
restores the legacy launcher scripts (run/ngps, run/_ngps) from their
permanently-tracked copies at watchdog_system/legacy/, then re-enables
ngps-daemon.timer/.service. The restore step is fully self-contained -- it
does not depend on the deploy host being on any particular git branch,
unlike a manual `git checkout main`. It does not remove the "ngpsops"
group or any user's membership in it -- group membership is left as a
durable host-level administrative fact, the same way the group itself is
never deleted.

uninstall-watchdog exists as a disaster-recovery/rollback tool -- rebuild
a corrupted host, back out a bad install-watchdog run -- not for routine
use. Once the watchdog system is validated in production, "uninstalling"
it makes as little sense as uninstalling any other daemon; the legacy
launcher scripts stay permanently tracked at watchdog_system/legacy/
specifically so that capability remains available indefinitely at near-
zero ongoing cost, without requiring the deploy host to juggle a separate
git ref.


------------------------------------------------------------------------------
Operating daemons -- systemctl directly
------------------------------------------------------------------------------
    All daemons      systemctl {start|stop|restart} ngps.target
    One daemon       systemctl {start|stop|restart} ngps@<name>.service
    Status           systemctl status ngps.target 'ngps@*'
    Instances list   systemctl list-units 'ngps@*'
    SIGKILL all      systemctl kill ngps.target          # cascades via PartOf=
    SIGKILL one      systemctl kill --signal=SIGKILL ngps@<name>.service

Operators do NOT need sudo for the commands above -- polkit grants them via
the "ngpsops" group. ngps-watchdog.service is intentionally NOT in the
polkit scope; controlling the watchdog requires admin auth.


------------------------------------------------------------------------------
Operating daemons -- the ngps wrapper (developer only)
------------------------------------------------------------------------------
watchdog_system/bin/ngps is a thin systemctl passthrough, installed by
install-watchdog to $RUN_DIR/ngps (default .../Software/run/ngps) with
execute permission restricted to user "developer". It saves typing during
development/debugging -- it is not installed to PATH and is not granted
to ngpsops:

    ngps {start|stop|restart} {all|<daemon> [<daemon> ...]}
    ngps status [<daemon> [<daemon> ...]]
    ngps {enable|disable}
    ngps list

"all" is required (never implied by a bare invocation) for start/stop/
restart, since omitting it would otherwise silently act on the whole
fleet. status has a well-defined bare-invocation default (matching
systemctl's own), so a daemon name narrows it and "all" is not a
recognized argument there. enable/disable take no argument at all --
ngps.target's own Wants= line pulls in every instance regardless of that
instance's own enable state, so a per-daemon enable/disable would not do
what it looks like; both always act on ngps.target only.


------------------------------------------------------------------------------
Verify (do all of these on a new system)
------------------------------------------------------------------------------
Boot chain is complete (without this, services show "enabled" yet stay
"inactive (dead)" after a reboot -- install-watchdog's `enable --now
ngps.target` step is what inserts the target into multi-user.target.wants/):
    systemctl is-enabled ngps.target                                    # -> enabled
    ls -l /etc/systemd/system/multi-user.target.wants/ngps.target       # must exist

All instances loaded and the aggregate is healthy:
    systemctl list-units 'ngps@*' --all
    systemctl status ngps.target 'ngps@*'

Sequencer starts only after its peers are READY (Type=notify ordering):
    systemd-analyze critical-chain ngps@sequencerd.service

Layer 1 -- auto-restart on death (kill one and watch it return ~2 s later):
    systemctl kill -s SIGKILL ngps@acamd.service ; sleep 3
    systemctl is-active ngps@acamd.service                 # -> active
    journalctl -u ngps@acamd.service -n 5 | grep -i 'scheduled restart'

Clean stop releases hardware and stays stopped (no auto-restart):
    systemctl stop ngps@acamd.service                          # operator: no sudo, polkit grants
    systemctl is-active ngps@acamd.service                     # -> inactive
    systemctl start ngps@acamd.service                         # bring it back

Operator cannot control the watchdog (admin auth required, by design):
    systemctl restart ngps-watchdog.service                    # -> polkit auth prompt

Liveness probe answers (port from BASEDIR/Config/sequencerd.cfg, e.g. ACAMD_PORT):
    printf 'ping\n' | nc -w2 127.0.0.1 <ACAMD_PORT>           # -> pong

Layer 2 -- watchdog restarts a FROZEN daemon (~30-60 s):
    journalctl -u ngps-watchdog -f          # in one terminal, watch it announce
    systemctl kill -s SIGSTOP ngps@acamd.service   # frozen but still "active"
    # expect: "ngps@acamd appears hung -> systemctl restart"
    systemctl is-active ngps@acamd.service          # -> active (fresh process)

Broker round-trip probe (messaged has no command port):
    systemctl kill -s SIGSTOP ngps@messaged.service  # -> watchdog restarts it

Sequencer-initiated restart works without a password prompt (as dataowner):
    sudo -u dataowner /usr/bin/systemctl restart ngps@acamd.service && echo OK
    # if it prompts for a password, install-watchdog's polkit rule
    # (10-ngps.rules) is not loaded; check:  journalctl -u polkit


------------------------------------------------------------------------------
Notes
------------------------------------------------------------------------------
- The ONLY thing that keeps a daemon down is a commanded stop
  (`systemctl stop`, and the stop half of `restart`). That is also how you
  halt a daemon that is crash-looping on bad config.

- KillSignal=SIGINT reuses each daemon's graceful shutdown. If a daemon does not
  release hardware within TimeoutStopSec=30, systemd will SIGKILL it -- verify
  each daemon's SIGINT path on first bring-up.

- During a hardware power-cycle the sequencer should `systemctl stop` the
  affected unit before cutting power and `start` it after, so Restart=always
  does not fight the power-off window.

- Where restarts are logged:
      journalctl -u ngps@acamd.service
      journalctl -u 'ngps@*' -b | grep -Ei 'restart|exited|started'
      journalctl -u ngps-watchdog -f

- Watchdog tunables (PROBE_PERIOD_SEC=10, PROBE_TIMEOUT_MS=5000,
  FAIL_THRESHOLD=3, COOLDOWN_SEC=120, STARTUP_GRACE_SEC=30) are compile-time
  constants at the top of watchdog_system/ngps_watchdog.cpp. The grace window
  holds off the first probe so a slow cold-boot is not mistaken for a hang;
  WatchdogSec= is heartbeated throughout it. If you change PROBE_PERIOD/timeout,
  keep WatchdogSec= in ngps-watchdog.service comfortably above one probe round.

==============================================================================
