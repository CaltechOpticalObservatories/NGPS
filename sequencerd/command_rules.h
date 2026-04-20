/**
 * @file     command_rules.h
 * @brief    rules for validating command parameters and transitioning states
 * @details  Each daemon has an enum of states, min/max number of parameters
 *           and a list of from-to states for each command. The daemon must
 *           be in the "from" state to accept the command, which then takes it
 *           to the "to" state.
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once
#include "command.h"
#include <vector>

namespace Sequencer {

  // ---------- CAMERAD --------------------------------------------------------

  enum class CameraState {
    IDLE,       ///< not yet opened
    READY,      ///< open and ready to accept commands
    EXPOSING,   ///< exposure in progress
    READING,    ///< readout in progress
    PAUSED      ///< exposure paused
  };

  const CommandSpecMap camerad_specs = {
    { CAMERAD_ABORT,         {0, 0} },
    { CAMERAD_ACTIVATE,      {0, 4} },   ///< 0 or more of: [ { U G R I } ]
    { CAMERAD_BIN,           {1, 2} },   ///< <axis> [ <binfactor> ]
    { CAMERAD_BOI,           {1,21} },   ///< <chan>|<dev#> [full|<nskip1> <nread1> [<nskip2> <nread2> [...]]]]
    { CAMERAD_CLOSE,         {0, 0} },
    { CAMERAD_DEACTIVATE,    {1, 4} },   ///< 1 or more of: { U G R I }
    { CAMERAD_EXPTIME,       {0, 1} },   ///< [ <exptime> ]
    { CAMERAD_EXPOSE,        {0, 0} },
    { CAMERAD_FITSNAME,      {0, 1} },
    { CAMERAD_FITSNAMING,    {0, 1} },   ///< [ time | number]
    { CAMERAD_FRAMETRANSFER, {1, 2} },   ///< <dev#> | <chan> | all [ yes | no ]
    { CAMERAD_GEOMETRY,      {1, 3} },   ///< <dev#> | <chan> [ <bytes> | <cols> <rows> ]
    { CAMERAD_IMDIR,         {0, 1} },   ///< [ <directory> ]
    { CAMERAD_IMNUM,         {0, 1} },   ///< [ <imnum> ]
    { CAMERAD_IMSIZE,        {1, 5} },   ///< <dev#>|<chan> [ <cols> <rows> <oscols> <osrows> ]
    { CAMERAD_KEY,           {1, 2} },   ///< list | KEYWORD=VALUE//COMMENT
    { CAMERAD_LOAD,          {0, 2} },   ///< [ <dev> <filename> ]
    { CAMERAD_MODEXPTIME,    {1, 1} },   ///< <exptime>
    { CAMERAD_OPEN,          {0, 4} },   ///< 0 or more of: [ { 0 1 2 3 } ]
    { CAMERAD_PAUSE,         {0, 0} },
    { CAMERAD_READOUT,       {0, 2} },
    { CAMERAD_RESUME,        {0, 0} },
    { CAMERAD_SHUTTER,       {0, 1} },   ///< [ enable | 1 | disable | 0 ]
    { CAMERAD_STOP,          {0, 0} }
  };

  const std::vector<Transition<CameraState>> camerad_transitions = {
    { CameraState::EXPOSING, CAMERAD_ABORT,         CameraState::READY    },
    { CameraState::READING,  CAMERAD_ABORT,         CameraState::READY    },
    { CameraState::PAUSED,   CAMERAD_ABORT,         CameraState::READY    },
    { CameraState::READY,    CAMERAD_ACTIVATE,      CameraState::READY    },
    { CameraState::READY,    CAMERAD_BIN,           CameraState::READY    },
    { CameraState::READY,    CAMERAD_BOI,           CameraState::READY    },
    { CameraState::READY,    CAMERAD_CLOSE,         CameraState::IDLE     },
    { CameraState::READY,    CAMERAD_DEACTIVATE,    CameraState::READY    },
    { CameraState::READY,    CAMERAD_EXPTIME,       CameraState::READY    },
    { CameraState::READY,    CAMERAD_EXPOSE,        CameraState::EXPOSING },
    { CameraState::READY,    CAMERAD_FITSNAME,      CameraState::READY    },
    { CameraState::READY,    CAMERAD_FITSNAMING,    CameraState::READY    },
    { CameraState::READY,    CAMERAD_FRAMETRANSFER, CameraState::READY    },
    { CameraState::READY,    CAMERAD_GEOMETRY,      CameraState::READY    },
    { CameraState::READY,    CAMERAD_IMDIR,         CameraState::READY    },
    { CameraState::READY,    CAMERAD_IMNUM,         CameraState::READY    },
    { CameraState::READY,    CAMERAD_IMSIZE,        CameraState::READY    },
    { CameraState::READY,    CAMERAD_KEY,           CameraState::READY    },
    { CameraState::READY,    CAMERAD_LOAD,          CameraState::READY    },
    { CameraState::EXPOSING, CAMERAD_MODEXPTIME,    CameraState::EXPOSING },
    { CameraState::IDLE,     CAMERAD_OPEN,          CameraState::READY    },
    { CameraState::EXPOSING, CAMERAD_PAUSE,         CameraState::PAUSED   },
    { CameraState::EXPOSING, CAMERAD_READOUT,       CameraState::READING  },
    { CameraState::READING,  CAMERAD_READOUT,       CameraState::READY    },
    { CameraState::PAUSED,   CAMERAD_RESUME,        CameraState::EXPOSING },
    { CameraState::READY,    CAMERAD_SHUTTER,       CameraState::READY    },
    { CameraState::EXPOSING, CAMERAD_STOP,          CameraState::READY    }
  };


  // ---------- ACAMD ----------------------------------------------------------

  enum class AcamState {
    IDLE,       ///< not yet opened
    READY,      ///< open and ready to accept commands
    ACQUIRING,  ///< target acquisition in progress
    GUIDING     ///< guiding in progress
  };

  const CommandSpecMap acamd_specs = {
    { ACAMD_OPEN,        {0, 2} },   ///< [ motion ] [ camera [<args>] ]
    { ACAMD_CLOSE,       {0, 0} },
    { ACAMD_INIT,        {0, 0} },
    { ACAMD_ACQUIRE,     {0, 4} },   ///< [ <ra> <dec> <angle> | target | guide | stop ]
    { ACAMD_COORDS,      {0, 3} },   ///< [ <ra> <dec> <angle> ]
    { ACAMD_EXPTIME,     {0, 1} },   ///< [ <exptime> ]
    { ACAMD_BIN,         {0, 2} },   ///< [ <hbin> <vbin> ]
    { ACAMD_GAIN,        {0, 1} },   ///< [ <gain> ]
    { ACAMD_TEMP,        {0, 1} },   ///< [ <setpoint> ]
    { ACAMD_FRAMEGRAB,   {1, 3} },   ///< start | stop | one [ <filename> ] | status
    { ACAMD_HOME,        {0, 1} },   ///< [ <name> ]
    { ACAMD_FILTER,      {0, 1} },   ///< [ <filtername> | home | ishome ]
    { ACAMD_COVER,       {0, 1} },   ///< [ open | close | home | ishome ]
    { ACAMD_ISOPEN,      {0, 1} },   ///< [ motion | camera ]
    { ACAMD_ISHOME,      {0, 1} },   ///< [ <name> ]
    { ACAMD_ISACQUIRED,  {0, 0} },
    { ACAMD_PUTONSLIT,   {0, 2} },   ///< [ <crossra> <crossdec> ]
    { ACAMD_SOLVE,       {0, 1} },   ///< [ <filename> ] [ <key>=<val> ... ] -- simplified
    { ACAMD_SHUTDOWN,    {0, 0} },
    { ACAMD_TCSINIT,     {0, 1} },   ///< [ tcs | sim ]
    { ACAMD_OFFSETGOAL,  {0, 2} },   ///< [ <dRA> <dDEC> ]
    { ACAMD_OFFSETPERIOD,{0, 1} }    ///< [ <sec> ]
  };

  const std::vector<Transition<AcamState>> acamd_transitions = {
    { AcamState::IDLE,      ACAMD_OPEN,       AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_CLOSE,      AcamState::IDLE      },  // TODO: verify
    { AcamState::READY,     ACAMD_INIT,       AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_ACQUIRE,    AcamState::ACQUIRING },  // TODO: verify
    { AcamState::READY,     ACAMD_COORDS,     AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_EXPTIME,    AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_BIN,        AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_GAIN,       AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_TEMP,       AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_FRAMEGRAB,  AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_HOME,       AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_FILTER,     AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_COVER,      AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_PUTONSLIT,  AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_SOLVE,      AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_OFFSETGOAL, AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_OFFSETPERIOD,AcamState::READY    },  // TODO: verify
    { AcamState::ACQUIRING, ACAMD_ACQUIRE,    AcamState::READY     },  // TODO: verify (stop)
    { AcamState::ACQUIRING, ACAMD_ISACQUIRED, AcamState::ACQUIRING },  // TODO: verify
    { AcamState::READY,     ACAMD_TCSINIT,    AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_SHUTDOWN,   AcamState::IDLE      },  // TODO: verify
    { AcamState::READY,     ACAMD_ISOPEN,     AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_ISHOME,     AcamState::READY     },  // TODO: verify
    { AcamState::READY,     ACAMD_ISACQUIRED, AcamState::READY     }   // TODO: verify
  };


  // ---------- CALIBD ---------------------------------------------------------

  enum class CalibState {
    IDLE,    ///< not yet opened
    READY,   ///< open and ready to accept commands
    MOVING   ///< actuator motion in progress
  };

  const CommandSpecMap calibd_specs = {
    { CALIBD_OPEN,    {0, 1} },   ///< [ motion | lampmod ]
    { CALIBD_CLOSE,   {0, 1} },   ///< [ motion | lampmod ]
    { CALIBD_ISOPEN,  {0, 1} },   ///< [ motion | lampmod ]
    { CALIBD_GET,     {0, 1} },   ///< [ <actuator> ]
    { CALIBD_HOME,    {0, 0} },
    { CALIBD_ISHOME,  {0, 0} },
    { CALIBD_SET,     {1, 8} },   ///< <actuator>=open|close ... (variable)
    { CALIBD_LAMPMOD, {1, 4} },   ///< open | close | reconnect | default | <n> ...
    { CALIBD_NATIVE,  {2, 2} }    ///< <addr> <cmd>
  };

  const std::vector<Transition<CalibState>> calibd_transitions = {
    { CalibState::IDLE,   CALIBD_OPEN,    CalibState::READY  },  // TODO: verify
    { CalibState::READY,  CALIBD_CLOSE,   CalibState::IDLE   },  // TODO: verify
    { CalibState::READY,  CALIBD_ISOPEN,  CalibState::READY  },  // TODO: verify
    { CalibState::READY,  CALIBD_GET,     CalibState::READY  },  // TODO: verify
    { CalibState::READY,  CALIBD_HOME,    CalibState::MOVING },  // TODO: verify
    { CalibState::MOVING, CALIBD_ISHOME,  CalibState::MOVING },  // TODO: verify
    { CalibState::READY,  CALIBD_ISHOME,  CalibState::READY  },  // TODO: verify
    { CalibState::READY,  CALIBD_SET,     CalibState::MOVING },  // TODO: verify
    { CalibState::MOVING, CALIBD_SET,     CalibState::READY  },  // TODO: verify (completion)
    { CalibState::READY,  CALIBD_LAMPMOD, CalibState::READY  },  // TODO: verify
    { CalibState::READY,  CALIBD_NATIVE,  CalibState::READY  }   // TODO: verify
  };


  // ---------- SLITD ----------------------------------------------------------

  enum class SlitState {
    IDLE,   ///< not yet opened
    READY,  ///< open and ready to accept commands
    MOVING, ///< slit motion in progress
    HOMED   ///< slit has been homed
  };

  const CommandSpecMap slitd_specs = {
    { SLITD_OPEN,   {0, 0} },
    { SLITD_CLOSE,  {0, 0} },
    { SLITD_ISOPEN, {0, 0} },
    { SLITD_GET,    {0, 1} },   ///< [ mm ]
    { SLITD_HOME,   {0, 0} },
    { SLITD_ISHOME, {0, 0} },
    { SLITD_OFFSET, {1, 1} },   ///< <offset>
    { SLITD_SET,    {1, 2} },   ///< <width> [ <offset> ]
    { SLITD_NATIVE, {2, 4} }    ///< <name> <cmd> [ <axis> <arg> ]
  };

  const std::vector<Transition<SlitState>> slitd_transitions = {
    { SlitState::IDLE,   SLITD_OPEN,   SlitState::READY  },  // TODO: verify
    { SlitState::READY,  SLITD_CLOSE,  SlitState::IDLE   },  // TODO: verify
    { SlitState::HOMED,  SLITD_CLOSE,  SlitState::IDLE   },  // TODO: verify
    { SlitState::READY,  SLITD_ISOPEN, SlitState::READY  },  // TODO: verify
    { SlitState::HOMED,  SLITD_ISOPEN, SlitState::HOMED  },  // TODO: verify
    { SlitState::READY,  SLITD_GET,    SlitState::READY  },  // TODO: verify
    { SlitState::HOMED,  SLITD_GET,    SlitState::HOMED  },  // TODO: verify
    { SlitState::READY,  SLITD_HOME,   SlitState::MOVING },  // TODO: verify
    { SlitState::MOVING, SLITD_ISHOME, SlitState::MOVING },  // TODO: verify (poll)
    { SlitState::MOVING, SLITD_HOME,   SlitState::HOMED  },  // TODO: verify (completion)
    { SlitState::READY,  SLITD_ISHOME, SlitState::READY  },  // TODO: verify
    { SlitState::HOMED,  SLITD_ISHOME, SlitState::HOMED  },  // TODO: verify
    { SlitState::HOMED,  SLITD_SET,    SlitState::MOVING },  // TODO: verify
    { SlitState::MOVING, SLITD_SET,    SlitState::HOMED  },  // TODO: verify (completion)
    { SlitState::HOMED,  SLITD_OFFSET, SlitState::MOVING },  // TODO: verify
    { SlitState::MOVING, SLITD_OFFSET, SlitState::HOMED  },  // TODO: verify (completion)
    { SlitState::HOMED,  SLITD_NATIVE, SlitState::HOMED  },  // TODO: verify
    { SlitState::READY,  SLITD_NATIVE, SlitState::READY  }   // TODO: verify
  };


  // ---------- TCSD -----------------------------------------------------------

  enum class TcsState {
    IDLE,       ///< not yet opened
    READY,      ///< open and ready to accept commands
    SLEWING,    ///< telescope slew in progress
    TRACKING,   ///< telescope tracking on target
    OFFSETTING  ///< telescope offset in progress
  };

  const CommandSpecMap tcsd_specs = {
    { TCSD_OPEN,           {1, 1} },   ///< <name>
    { TCSD_CLOSE,          {0, 0} },
    { TCSD_ISOPEN,         {0, 0} },
    { TCSD_COORDS,         {5, 7} },   ///< <ra> <dec> <equinox> <ramotion> <decmotion> [<motionflag>] ["<name>"]
    { TCSD_GET_COORDS,     {0, 0} },
    { TCSD_GET_CASS,       {0, 0} },
    { TCSD_GET_DOME,       {0, 0} },
    { TCSD_GET_FOCUS,      {0, 0} },
    { TCSD_GET_MOTION,     {0, 0} },
    { TCSD_GET_OFFSETS,    {0, 0} },
    { TCSD_GET_PA,         {0, 0} },
    { TCSD_GET_NAME,       {0, 0} },
    { TCSD_RINGGO,         {1, 1} },   ///< <angle>
    { TCSD_PTOFFSET,       {2, 2} },   ///< <ra> <dec>
    { TCSD_RETOFFSETS,     {0, 0} },
    { TCSD_ZERO_OFFSETS,   {0, 0} },
    { TCSD_OFFSETRATE,     {0, 2} },   ///< [ <raoff> <decoff> ]
    { TCSD_SET_FOCUS,      {1, 1} },   ///< <value>
    { TCSD_NATIVE,         {1, 1} },   ///< <cmd>
    { TCSD_LIST,           {0, 0} },
    { TCSD_LLIST,          {0, 0} },
    { TCSD_WEATHER_COORDS, {0, 0} }
  };

  const std::vector<Transition<TcsState>> tcsd_transitions = {
    { TcsState::IDLE,       TCSD_OPEN,           TcsState::READY      },  // TODO: verify
    { TcsState::READY,      TCSD_CLOSE,          TcsState::IDLE       },  // TODO: verify
    { TcsState::TRACKING,   TCSD_CLOSE,          TcsState::IDLE       },  // TODO: verify
    { TcsState::READY,      TCSD_ISOPEN,         TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_ISOPEN,         TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_COORDS,         TcsState::SLEWING    },  // TODO: verify
    { TcsState::TRACKING,   TCSD_COORDS,         TcsState::SLEWING    },  // TODO: verify
    { TcsState::SLEWING,    TCSD_GET_MOTION,     TcsState::SLEWING    },  // TODO: verify (poll)
    { TcsState::SLEWING,    TCSD_GET_COORDS,     TcsState::TRACKING   },  // TODO: verify (completion)
    { TcsState::READY,      TCSD_GET_COORDS,     TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_GET_COORDS,     TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_GET_CASS,       TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_GET_CASS,       TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_GET_DOME,       TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_GET_DOME,       TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_GET_FOCUS,      TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_GET_FOCUS,      TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_GET_MOTION,     TcsState::READY      },  // TODO: verify
    { TcsState::READY,      TCSD_GET_OFFSETS,    TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_GET_OFFSETS,    TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_GET_PA,         TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_GET_PA,         TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_GET_NAME,       TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_GET_NAME,       TcsState::TRACKING   },  // TODO: verify
    { TcsState::TRACKING,   TCSD_RINGGO,         TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_RINGGO,         TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_PTOFFSET,       TcsState::OFFSETTING },  // TODO: verify
    { TcsState::OFFSETTING, TCSD_GET_MOTION,     TcsState::OFFSETTING },  // TODO: verify (poll)
    { TcsState::OFFSETTING, TCSD_RETOFFSETS,     TcsState::TRACKING   },  // TODO: verify (completion)
    { TcsState::TRACKING,   TCSD_RETOFFSETS,     TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_ZERO_OFFSETS,   TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_ZERO_OFFSETS,   TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_OFFSETRATE,     TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_OFFSETRATE,     TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_SET_FOCUS,      TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_SET_FOCUS,      TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_NATIVE,         TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_NATIVE,         TcsState::TRACKING   },  // TODO: verify
    { TcsState::READY,      TCSD_LIST,           TcsState::READY      },  // TODO: verify
    { TcsState::READY,      TCSD_LLIST,          TcsState::READY      },  // TODO: verify
    { TcsState::READY,      TCSD_WEATHER_COORDS, TcsState::READY      },  // TODO: verify
    { TcsState::TRACKING,   TCSD_WEATHER_COORDS, TcsState::TRACKING   }   // TODO: verify
  };


  // ---------- FOCUSD ---------------------------------------------------------

  enum class FocusState {
    IDLE,   ///< not yet opened
    READY,  ///< open and ready to accept commands
    MOVING  ///< focus motion in progress
  };

  const CommandSpecMap focusd_specs = {
    { FOCUSD_OPEN,       {0, 0} },
    { FOCUSD_CLOSE,      {0, 0} },
    { FOCUSD_ISOPEN,     {0, 0} },
    { FOCUSD_GET,        {1, 1} },   ///< <chan>
    { FOCUSD_HOME,       {0, 1} },   ///< [ <chan> ]
    { FOCUSD_ISHOME,     {0, 0} },
    { FOCUSD_SET,        {2, 2} },   ///< <chan> { <pos> | nominal }
    { FOCUSD_DEFAULTPOS, {0, 0} },
    { FOCUSD_NATIVE,     {2, 2} },   ///< <chan> <cmd>
    { FOCUSD_TEST,       {1, 4} }    ///< <testname> ...
  };

  const std::vector<Transition<FocusState>> focusd_transitions = {
    { FocusState::IDLE,   FOCUSD_OPEN,       FocusState::READY  },  // TODO: verify
    { FocusState::READY,  FOCUSD_CLOSE,      FocusState::IDLE   },  // TODO: verify
    { FocusState::READY,  FOCUSD_ISOPEN,     FocusState::READY  },  // TODO: verify
    { FocusState::READY,  FOCUSD_GET,        FocusState::READY  },  // TODO: verify
    { FocusState::READY,  FOCUSD_HOME,       FocusState::MOVING },  // TODO: verify
    { FocusState::MOVING, FOCUSD_HOME,       FocusState::READY  },  // TODO: verify (completion)
    { FocusState::READY,  FOCUSD_ISHOME,     FocusState::READY  },  // TODO: verify
    { FocusState::MOVING, FOCUSD_ISHOME,     FocusState::MOVING },  // TODO: verify (poll)
    { FocusState::READY,  FOCUSD_SET,        FocusState::MOVING },  // TODO: verify
    { FocusState::MOVING, FOCUSD_SET,        FocusState::READY  },  // TODO: verify (completion)
    { FocusState::READY,  FOCUSD_DEFAULTPOS, FocusState::MOVING },  // TODO: verify
    { FocusState::MOVING, FOCUSD_DEFAULTPOS, FocusState::READY  },  // TODO: verify (completion)
    { FocusState::READY,  FOCUSD_NATIVE,     FocusState::READY  },  // TODO: verify
    { FocusState::READY,  FOCUSD_TEST,       FocusState::READY  }   // TODO: verify
  };


  // ---------- FLEXURED -------------------------------------------------------

  enum class FlexureState {
    IDLE,   ///< not yet opened
    READY,  ///< open and ready to accept commands
    MOVING  ///< flexure compensation motion in progress
  };

  const CommandSpecMap flexured_specs = {
    { FLEXURED_OPEN,        {0, 0} },
    { FLEXURED_CLOSE,       {0, 0} },
    { FLEXURED_ISOPEN,      {0, 0} },
    { FLEXURED_GET,         {2, 2} },   ///< <chan> <axis>
    { FLEXURED_SET,         {3, 3} },   ///< <chan> <axis> <pos>
    { FLEXURED_COMPENSATE,  {0, 1} },   ///< [ dryrun ]
    { FLEXURED_DEFAULTPOS,  {0, 0} },
    { FLEXURED_NATIVE,      {2, 2} },   ///< <chan> <cmd>
    { FLEXURED_TEST,        {1, 4} }    ///< <testname> ...
  };

  const std::vector<Transition<FlexureState>> flexured_transitions = {
    { FlexureState::IDLE,   FLEXURED_OPEN,       FlexureState::READY  },  // TODO: verify
    { FlexureState::READY,  FLEXURED_CLOSE,      FlexureState::IDLE   },  // TODO: verify
    { FlexureState::READY,  FLEXURED_ISOPEN,     FlexureState::READY  },  // TODO: verify
    { FlexureState::READY,  FLEXURED_GET,        FlexureState::READY  },  // TODO: verify
    { FlexureState::READY,  FLEXURED_SET,        FlexureState::MOVING },  // TODO: verify
    { FlexureState::MOVING, FLEXURED_SET,        FlexureState::READY  },  // TODO: verify (completion)
    { FlexureState::READY,  FLEXURED_COMPENSATE, FlexureState::MOVING },  // TODO: verify
    { FlexureState::MOVING, FLEXURED_COMPENSATE, FlexureState::READY  },  // TODO: verify (completion)
    { FlexureState::READY,  FLEXURED_DEFAULTPOS, FlexureState::MOVING },  // TODO: verify
    { FlexureState::MOVING, FLEXURED_DEFAULTPOS, FlexureState::READY  },  // TODO: verify (completion)
    { FlexureState::READY,  FLEXURED_NATIVE,     FlexureState::READY  },  // TODO: verify
    { FlexureState::READY,  FLEXURED_TEST,       FlexureState::READY  }   // TODO: verify
  };


  // ---------- POWERD ---------------------------------------------------------

  enum class PowerState {
    IDLE,  ///< not yet opened
    READY  ///< open and ready to accept commands
  };

  const CommandSpecMap powerd_specs = {
    { POWERD_OPEN,   {0, 0} },
    { POWERD_CLOSE,  {0, 0} },
    { POWERD_ISOPEN, {0, 0} },
    { POWERD_STATUS, {0, 1} },   ///< [ ? ]
    { POWERD_REOPEN, {0, 0} },
    { POWERD_LIST,   {0, 0} }
    // NOTE: plug switching commands (<unit> <plug> ON|OFF|BOOT or <plugname> ON|OFF|BOOT)
    // do not have a fixed first-word command name and so cannot be registered here by name.
    // TODO: decide how plug switching is exposed via SEQUENCERD_OP.
  };

  const std::vector<Transition<PowerState>> powerd_transitions = {
    { PowerState::IDLE,  POWERD_OPEN,   PowerState::READY },  // TODO: verify
    { PowerState::READY, POWERD_CLOSE,  PowerState::IDLE  },  // TODO: verify
    { PowerState::READY, POWERD_ISOPEN, PowerState::READY },  // TODO: verify
    { PowerState::READY, POWERD_STATUS, PowerState::READY },  // TODO: verify
    { PowerState::READY, POWERD_REOPEN, PowerState::READY },  // TODO: verify
    { PowerState::READY, POWERD_LIST,   PowerState::READY }   // TODO: verify
  };


  // ---------- SLICECAMD ------------------------------------------------------

  enum class SlicecamState {
    IDLE,       ///< not yet opened
    READY,      ///< open and ready to accept commands
    ACQUIRING,  ///< fine acquisition in progress
    GUIDING     ///< guiding in progress
  };

  const CommandSpecMap slicecamd_specs = {
    { SLICECAMD_OPEN,        {0, 2} },   ///< [ motion ] [ camera [<args>] ]
    { SLICECAMD_CLOSE,       {0, 0} },
    { SLICECAMD_ISOPEN,      {0, 1} },   ///< [ motion | camera ]
    { SLICECAMD_INIT,        {0, 0} },
    { SLICECAMD_EXPTIME,     {0, 1} },   ///< [ <exptime> ]
    { SLICECAMD_BIN,         {0, 3} },   ///< [ <name> <hbin> <vbin> ]
    { SLICECAMD_GAIN,        {0, 1} },   ///< [ <gain> ]
    { SLICECAMD_TEMP,        {0, 1} },   ///< [ <setpoint> ]
    { SLICECAMD_FRAMEGRAB,   {1, 3} },   ///< start | stop | one [ <filename> ] | status
    { SLICECAMD_FINEACQUIRE, {1, 2} },   ///< status | stop | start { L | R }
    { SLICECAMD_PUTONSLIT,   {0, 4} },   ///< [ <slitra> <slitdec> <crossra> <crossdec> ]
    { SLICECAMD_SAVEFRAMES,  {0, 2} },   ///< [ <nsave> <nskip> ]
    { SLICECAMD_SHUTTER,     {1, 1} },   ///< open | close | auto
    { SLICECAMD_SHUTDOWN,    {0, 0} },
    { SLICECAMD_TCSINIT,     {0, 1} },   ///< [ tcs | sim ]
    { SLICECAMD_IMFLIP,      {0, 3} },   ///< [ <name> <hflip> <vflip> ]
    { SLICECAMD_IMROT,       {0, 2} }    ///< [ <name> <rotdir> ]
  };

  const std::vector<Transition<SlicecamState>> slicecamd_transitions = {
    { SlicecamState::IDLE,      SLICECAMD_OPEN,        SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_CLOSE,       SlicecamState::IDLE      },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_ISOPEN,      SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_INIT,        SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_EXPTIME,     SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_BIN,         SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_GAIN,        SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_TEMP,        SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_FRAMEGRAB,   SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_FINEACQUIRE, SlicecamState::ACQUIRING },  // TODO: verify
    { SlicecamState::ACQUIRING, SLICECAMD_FINEACQUIRE, SlicecamState::READY     },  // TODO: verify (stop)
    { SlicecamState::READY,     SLICECAMD_PUTONSLIT,   SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_SAVEFRAMES,  SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_SHUTTER,     SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_SHUTDOWN,    SlicecamState::IDLE      },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_TCSINIT,     SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_IMFLIP,      SlicecamState::READY     },  // TODO: verify
    { SlicecamState::READY,     SLICECAMD_IMROT,       SlicecamState::READY     }   // TODO: verify
  };

}
