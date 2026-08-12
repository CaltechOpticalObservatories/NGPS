# Software Build And Configuration

This section describes how to build, configure, and deploy the NGPS software stack on a new or replacement instrument control computer. It is intended for software developers and system administrators; observers do not normally need this information.

## Machine Build Setup

The instrument control computer runs CentOS-8.5.2111. A new or rebuilt machine should be provisioned with this OS release before any NGPS software is installed, so that package versions and system libraries match the supported build environment.

The following base packages and tools are required on the build machine:

- CentOS-8.5.2111 (base OS)
- git
- Python 3 and pip
- Docker / container runtime
- Build toolchain (gcc/g++, make, cmake)

A complete, versioned list of required packages is maintained in the NGPS repository and should be treated as the source of truth if it differs from the list above.

## Pulling Software from GitHub

NGPS software is version-controlled in GitHub at <https://github.com/CaltechOpticalObservatories/NGPS>.

## Instrument Configuration Setup

Once the software is built and installed, each daemon requires its configuration file to be set up for the specific instrument computer and observatory environment (see also {doc}`software-architecture` for how observers edit user-level configuration values at the telescope).

## Deployment

NGPS software is built and deployed using a GitHub Actions workflow together with TeamCity. On a push or merge to the designated deployment branch, the GitHub workflow builds and tests the software; TeamCity then packages and deploys the resulting build to the instrument control computer.
