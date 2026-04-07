
#pragma once

#include "command.h"

#include <vector>

namespace Sequencer {

  enum class CameraState {
    IDLE,
    READY,
    EXPOSING,
    READING
   };

  const CommandSpecMap camerad_specs = {
    { CAMERAD_ACTIVATE,   {0, 4} },
    { CAMERAD_DEACTIVATE, {1, 4} },
    { CAMERAD_OPEN,       {0, 1} },
    { CAMERAD_CLOSE,      {0, 0} },
    { CAMERAD_EXPTIME,    {0, 1} },
    { CAMERAD_EXPOSE,     {0, 0} },
    { CAMERAD_READOUT,    {0, 2} }
  };

  const std::vector<Transition<CameraState>> camerad_transitions = {
    { CameraState::IDLE,     CAMERAD_OPEN,       CameraState::READY },
    { CameraState::READY,    CAMERAD_ACTIVATE,   CameraState::READY },
    { CameraState::READY,    CAMERAD_DEACTIVATE, CameraState::READY },
    { CameraState::READY,    CAMERAD_EXPTIME,    CameraState::READY },
    { CameraState::READY,    CAMERAD_EXPOSE,     CameraState::EXPOSING },
    { CameraState::EXPOSING, CAMERAD_READOUT,    CameraState::READING },
    { CameraState::READING,  CAMERAD_READOUT,    CameraState::READY }
  };

}
