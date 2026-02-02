#!/usr/bin/env bash

# NGPS local environment helper. Source this from your shell:
#   source /path/to/NGPS/env.sh

SCRIPT_DIR="$(cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd)"

export NGPS_ROOT="${NGPS_ROOT:-${SCRIPT_DIR}}"
export NGPS_DATA_DIR="${NGPS_DATA_DIR:-${NGPS_ROOT}/data}"

# Prefer repo run/bin wrappers
export PATH="${NGPS_ROOT}/run:${NGPS_ROOT}/bin:${PATH}"

# Skymaker (sky) executable for skysim
if [ -x "/opt/homebrew/bin/sky" ]; then
  export NGPS_SKYEXEC="/opt/homebrew/bin/sky"
fi

# Python tools used by acquisition/skyinfo
VENV_SITEPKG="${HOME}/venvs/ngps/lib/python3.14/site-packages"
if [ -n "${PYTHONPATH}" ]; then
  export PYTHONPATH="${NGPS_ROOT}/Python:${NGPS_ROOT}/Python/acam_skyinfo:${VENV_SITEPKG}:${PYTHONPATH}"
else
  export PYTHONPATH="${NGPS_ROOT}/Python:${NGPS_ROOT}/Python/acam_skyinfo:${VENV_SITEPKG}"
fi

# Allow XPA access to DS9 windows
export XPA_NSUSERS="*"

# Optional: add DS9 to PATH if installed in the default macOS location
if [ -d "/Applications/SAOImageDS9.app/Contents/Resources/bin" ]; then
  export PATH="/Applications/SAOImageDS9.app/Contents/Resources/bin:${PATH}"
fi

# Optional: add conda SExtractor (sex) if installed there
if [ -x "${HOME}/miniconda3/bin/sex" ]; then
  export PATH="${HOME}/miniconda3/bin:${PATH}"
fi
