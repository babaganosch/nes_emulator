# Download genie binaries if missing
if [ ! -f tools/linux/genie ]; then
    mkdir -p tools/linux/
    wget https://github.com/bkaradzic/bx/raw/master/tools/bin/linux/genie -O tools/linux/genie
    chmod +x tools/linux/genie
fi

if [ ! -f tools/macos/genie ]; then
    mkdir -p tools/macos/
    wget https://github.com/bkaradzic/bx/raw/master/tools/bin/darwin/genie -O tools/macos/genie
    chmod +x tools/macos/genie
fi

if [ ! -f tools/windows/genie.exe ]; then
    mkdir -p tools/windows/
    wget https://github.com/bkaradzic/bx/raw/master/tools/bin/windows/genie.exe -O tools/windows/genie.exe
    chmod +x tools/windows/genie.exe
fi

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

# Open new shell to get correct env vars
$SHELL