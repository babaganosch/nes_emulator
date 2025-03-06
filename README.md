# How to build

1. git submodule update --recursive --init
2. ./setup.sh
3. ./generate.sh
4. cd build
5. make


# Building on Ubuntu:

* Rebuild genie locally if it complains about being built with newer GLIBC version
* X11 development libraries might be needed
```sudo apt-get install libx11-dev```
* XKB development libraries might be needed
```sudo apt-get install libxkbcommon-dev```