# Setup env vars
export _WORKSPACE_HOME="`pwd`"
if [ "$(uname)" == "Darwin" ]; then
    export _WORKSPACE_PLATFORM="macos"
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    export _WORKSPACE_PLATFORM="windows"
else
    export _WORKSPACE_PLATFORM="linux"
fi
export PATH=$PATH:$_WORKSPACE_HOME/tools/$_WORKSPACE_PLATFORM/


# Download genie & ninja binaries if missing
if [ $_WORKSPACE_PLATFORM == "linux" ]; then
    if [ ! -f tools/linux/genie ]; then
        mkdir -p tools/linux/
        curl -L https://github.com/bkaradzic/bx/raw/master/tools/bin/linux/genie -o tools/linux/genie
        chmod +x tools/linux/genie
    fi
    if [ ! -f tools/linux/ninja ]; then
        mkdir -p tools/linux/
        curl -L https://github.com/ninja-build/ninja/releases/latest/download/ninja-linux.zip -o tools/linux/ninja-linux.zip
        cd tools/linux
        tar -xzf ninja-linux.zip
        rm ninja-linux.zip
        cd -
    fi
fi

if [ $_WORKSPACE_PLATFORM == "macos" ]; then
    if [ ! -f tools/macos/genie ]; then
        mkdir -p tools/macos/
        curl -L https://github.com/bkaradzic/bx/raw/master/tools/bin/darwin/genie -o tools/macos/genie
        chmod +x tools/macos/genie
    fi
    if [ ! -f tools/macos/ninja ]; then
        mkdir -p tools/macos/
        curl -L https://github.com/ninja-build/ninja/releases/latest/download/ninja-mac.zip -o tools/macos/ninja-mac.zip
        cd tools/macos
        tar -xzf ninja-mac.zip
        rm ninja-mac.zip
        cd -
    fi
fi

if [ $_WORKSPACE_PLATFORM == "windows" ]; then
    if [ ! -f tools/windows/genie.exe ]; then
        mkdir -p tools/windows/
        curl -L https://github.com/bkaradzic/bx/raw/master/tools/bin/windows/genie.exe -o tools/windows/genie.exe
        chmod +x tools/windows/genie.exe
    fi
    if [ ! -f tools/windows/ninja.exe ]; then
        mkdir -p tools/windows/
        curl -L https://github.com/ninja-build/ninja/releases/latest/download/ninja-win.zip -o tools/windows/ninja-win.zip
        cd tools/windows
        unzip ninja-win.zip
        rm ninja-win.zip
        cd -
    fi
fi

# Open new shell to get correct env vars
$SHELL