#!/bin/bash

# Exit on error
set -e

# Get the directory where the script is located
_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
_WORKSPACE_HOME="$( cd "$_SCRIPT_DIR/.." && pwd )"

# Determine the platform
if [[ "$OSTYPE" == "darwin"* ]]; then
    _WORKSPACE_PLATFORM="macos"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    _WORKSPACE_PLATFORM="windows"
else
    _WORKSPACE_PLATFORM="linux"
fi

# Add tools directory to PATH
if [ -z "$PATH" ]; then
    export PATH=$_WORKSPACE_HOME/tools/$_WORKSPACE_PLATFORM/
else
    export PATH=$PATH:$_WORKSPACE_HOME/tools/$_WORKSPACE_PLATFORM/
fi

# Check for required tools
command -v curl >/dev/null 2>&1 || { echo "Error: curl is required but not installed. Aborting." >&2; exit 1; }
if [ $_WORKSPACE_PLATFORM == "windows" ]; then
    command -v unzip >/dev/null 2>&1 || { echo "Error: unzip is required but not installed. Aborting." >&2; exit 1; }
    command -v make >/dev/null 2>&1 || { echo "Error: make is required but not installed. Please install MinGW or MSYS2 which includes make." >&2; exit 1; }
else
    command -v tar >/dev/null 2>&1 || { echo "Error: tar is required but not installed. Aborting." >&2; exit 1; }
    command -v make >/dev/null 2>&1 || { echo "Error: make is required but not installed. Aborting." >&2; exit 1; }
fi

# Create tools directory
mkdir -p tools/$_WORKSPACE_PLATFORM/


# Download genie & ninja binaries if missing
if [ $_WORKSPACE_PLATFORM == "linux" ]; then
    if [ ! -f tools/linux/genie ]; then
        curl -L https://github.com/bkaradzic/bx/raw/master/tools/bin/linux/genie -o tools/linux/genie
        chmod +x tools/linux/genie
    fi
    if [ ! -f tools/linux/ninja ]; then
        curl -L https://github.com/ninja-build/ninja/releases/latest/download/ninja-linux.zip -o tools/linux/ninja-linux.zip
        cd tools/linux
        tar -xzf ninja-linux.zip
        rm ninja-linux.zip
        cd -
    fi
fi

if [ $_WORKSPACE_PLATFORM == "macos" ]; then
    if [ ! -f tools/macos/genie ]; then
        curl -L https://github.com/bkaradzic/bx/raw/master/tools/bin/darwin/genie -o tools/macos/genie
        chmod +x tools/macos/genie
    fi
    if [ ! -f tools/macos/ninja ]; then
        curl -L https://github.com/ninja-build/ninja/releases/latest/download/ninja-mac.zip -o tools/macos/ninja-mac.zip
        cd tools/macos
        tar -xzf ninja-mac.zip
        rm ninja-mac.zip
        cd -
    fi
fi

if [ $_WORKSPACE_PLATFORM == "windows" ]; then
    if [ ! -f tools/windows/genie.exe ]; then
        curl -L https://github.com/bkaradzic/bx/raw/master/tools/bin/windows/genie.exe -o tools/windows/genie.exe
        chmod +x tools/windows/genie.exe
    fi
    if [ ! -f tools/windows/ninja.exe ]; then
        curl -L https://github.com/ninja-build/ninja/releases/latest/download/ninja-win.zip -o tools/windows/ninja-win.zip
        cd tools/windows
        unzip ninja-win.zip
        rm ninja-win.zip
        cd -
    fi
fi

# Open new shell to get correct env vars
$SHELL