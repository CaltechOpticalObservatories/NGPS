# @file     readme.txt
# @brief    readme file for ngps watchdog system
# @details  how to set up the watchdog 
# @author   David Hale
#

==============================================================================
NGPS daemon system control + watchdog -- system setup checklist
==============================================================================

The three safety nets:
  Layer 1  systemd restarts any daemon that EXITS (crash/signal/exit 0).
  Layer 2  ngps-watchdog restarts any daemon that is alive but FROZEN.
  Layer 3  systemd watches ngps-watchdog itself (WatchdogSec heartbeat).
Plus: the sequencer restarts a peer it cannot talk to, via systemctl.


------------------------------------------------------------------------------
0. Installation
------------------------------------------------------------------------------
BASEDIR : /home/developer/Software  (binaries in BASEDIR/bin, configs in BASEDIR/Config).

Service account / group: user "dataowner", group "datawriters".
  - daemons run as this user/group
  - the sequencer runs as dataowner and restarts peers via systemctl

Operator group: "ngpsops".
  - members of "ngpsops" may run `systemctl {start|stop|restart} ngps.target`
    and `systemctl ... ngps@<name>.service` without a password (per polkit
    rule, see step 3). They cannot control ngps-watchdog.service -- that one
    requires admin auth, by design.
  - datawriters is for write access into the data tree and must NOT be used
    to grant daemon-control rights -- keep the two groups distinct.

Create the accounts/groups if they do not exist, then add human operators
(e.g. observer) to ngpsops:
    sudo groupadd ngpsops 2>/dev/null || true
    sudo usermod -aG ngpsops observer
Operators must log out and back in for the new group membership to take effect.


------------------------------------------------------------------------------
1. Install the systemd units
------------------------------------------------------------------------------
All install commands in steps 1-3 are run from the watchdog_system/ directory.

The files to install live in this directory tree (watchdog_system/):

    watchdog_system/systemd/ngps@.service
    watchdog_system/systemd/ngps.target
    watchdog_system/systemd/ngps-watchdog.service
    watchdog_system/systemd/ngps@sequencerd.service.d/order.conf
    watchdog_system/systemd/ngps.env
    watchdog_system/polkit/10-ngps.rules

Install them:

    sudo install -m 0644 systemd/ngps@.service        /etc/systemd/system/ngps@.service
    sudo install -m 0644 systemd/ngps.target          /etc/systemd/system/ngps.target
    sudo install -m 0644 systemd/ngps-watchdog.service /etc/systemd/system/ngps-watchdog.service

    sudo mkdir -p /etc/systemd/system/ngps@sequencerd.service.d
    sudo install -m 0644 systemd/ngps@sequencerd.service.d/order.conf \
         /etc/systemd/system/ngps@sequencerd.service.d/order.conf


------------------------------------------------------------------------------
2. Install the environment file
------------------------------------------------------------------------------
    sudo install -m 0644 systemd/ngps.env /etc/sysconfig/ngps

Then edit /etc/sysconfig/ngps so PYTHONPATH / RUBIN_SIM_DATA_DIR / XPA_NSUSERS
match the target.


------------------------------------------------------------------------------
3. Install the polkit rule (passwordless systemctl for ngps* units only)
------------------------------------------------------------------------------
Lets user "dataowner" (the sequencer + watchdog) and group "ngpsops"
(interactive operators) manage ngps* units without a password.

    sudo install -m 0644 polkit/10-ngps.rules /etc/polkit-1/rules.d/10-ngps.rules


------------------------------------------------------------------------------
4. Disable any old boot mechanism (if migrating from the single-service setup)
------------------------------------------------------------------------------
    sudo systemctl disable --now ngps-daemon.timer    2>/dev/null || true
    sudo systemctl disable --now ngps-daemon.service   2>/dev/null || true


------------------------------------------------------------------------------
5. Reload, enable, and start everything
------------------------------------------------------------------------------
    sudo systemctl daemon-reload
    sudo systemctl enable --now ngps.target            # Layer 1: all daemons
    sudo systemctl enable --now ngps-watchdog.service  # Layer 2 + 3: hang watch


------------------------------------------------------------------------------
6. Verify (do all of these on a new system)
------------------------------------------------------------------------------
Boot chain is complete (without this, services show "enabled" yet stay
"inactive (dead)" after a reboot -- step 5's `enable --now ngps.target` is what
inserts the target into multi-user.target.wants/):
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
    # if it prompts for a password, the polkit rule (step 3) is not loaded;
    # check:  journalctl -u polkit


------------------------------------------------------------------------------
Operator command reference (systemctl is the canonical control surface;
no `ngps` wrapper is installed in PATH)
------------------------------------------------------------------------------
    All daemons      systemctl {start|stop|restart} ngps.target
    One daemon       systemctl {start|stop|restart} ngps@<name>.service
    Status           systemctl status ngps.target 'ngps@*'
    Instances list   systemctl list-units 'ngps@*'
    SIGKILL all      systemctl kill ngps.target          # cascades via PartOf=
    SIGKILL one      systemctl kill --signal=SIGKILL ngps@<name>.service

Operators do NOT need sudo for the commands above -- polkit grants them via
the "ngpsops" group. ngps-watchdog.service is intentionally NOT in the polkit
scope; controlling the watchdog requires admin auth.

A developer-only wrapper lives at watchdog_system/bin/ngps for convenience
when debugging from that directory. It is not installed to PATH.


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
