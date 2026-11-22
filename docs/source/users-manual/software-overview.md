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

On the instrument computers the repository is checked out at `/home/developer/Software`, and the C++ software is built in `/home/developer/Software/build`.

## Instrument Configuration Setup

Once the software is built and installed, each daemon requires its configuration file to be set up for the specific instrument computer and observatory environment (see also {doc}`software-architecture` for how observers edit user-level configuration values at the telescope).

## Deployment

NGPS software is built and deployed using a GitHub Actions workflow together with TeamCity. On a push or merge to the designated deployment branch, the GitHub workflow builds and tests the software; TeamCity then packages and deploys the resulting build to the instrument control computer.

### Release Tags

Each release is an annotated git tag named `vX.Y.Z` (for example `v1.10.1`). The tag message holds the release notes, normally the list of pull requests merged since the previous tag:

```text
Release version 1.10.1

Merged pull requests (since <previous tag commit>):

- #516 PAL-172 Fix targetlist indexing (prkrtg)
- ...

Full changelog: https://github.com/CaltechOpticalObservatories/NGPS/compare/v1.10.0...v1.10.1
```

To create and publish a tag from the latest `main`:

```text
git fetch origin
git tag -a v1.10.1 origin/main -F notes.txt
git push origin v1.10.1
```

```{important}
Pushing a `vX.Y.Z` tag triggers the `release-slack.yml` GitHub workflow, which posts the tag message to Slack as the release announcement. Check the version number and notes before pushing.
```

### Deploying a Release Tag Manually

The `run/ngps-deploy` script checks out a release tag on an instrument computer, rebuilds the software, and restarts NGPS. It can deploy the newest tag, roll back to the previous tag, or deploy any specific tag.

Install a copy outside the repository so the script is still available after rolling back to a tag that predates it:

```text
cp /home/developer/Software/run/ngps-deploy ~/bin/
```

Usage:

```text
ngps-deploy                 # interactive menu: latest / roll back / pick a tag
ngps-deploy latest          # deploy the newest vX.Y.Z tag
ngps-deploy rollback        # deploy the tag before the one currently checked out
ngps-deploy tag v1.10.0     # deploy a specific tag
ngps-deploy list            # show what is checked out and the recent tags

Options:  -y  skip the confirmation prompt
          -n  dry run: print the steps without changing anything
```

After fetching tags from GitHub and checking out the selected tag, the script runs:

```text
ngps stop
cd /home/developer/Software/build
cmake -DINSTR=generic-arc ..
make
ngps start
```

The repository path, build directory, and `INSTR` value can be overridden with the `NGPS_REPO`, `BUILD_DIR`, and `INSTR` environment variables.

Before it stops NGPS, the script shows the current and target versions and asks for confirmation. It refuses to run if tracked files in the repository have uncommitted changes, or if the tag does not exist. Each deploy, successful or failed, is recorded in `~/.ngps-deploy.log`.

```{caution}
NGPS is stopped while the software builds. If `cmake` or `make` fails, the script does **not** restart NGPS. It reports which step failed and prints the command to return to the previously deployed version (for example `ngps-deploy tag v1.10.0`). Run that command to rebuild and restart the previous version.
```

#### Rolling Back

`ngps-deploy rollback` deploys the tag immediately before the one currently checked out. If the checkout is not exactly on a tag (for example, a branch ahead of the last release), it deploys the most recent tag below the current commit. Running `rollback` again steps back one more release; `ngps-deploy latest` returns to the newest release.

Run `ngps-deploy list` at any time to see which version is checked out.
