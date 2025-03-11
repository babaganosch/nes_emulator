# NesScape

A Nintendo Entertainment System (NES) emulator implementation in C++ that aims to provide cycle accurate emulation of the original NTSC NES hardware. Implemented and tested on MacOS, but should _probably_ work on Windows and Linux as well.

## Features

__CPU__
- All 256 opcodes implemented and cycle accurate
- BRK, IRQ and NMI implemented with vector hijacking

__PPU__
- Accurate pixel rendering
- Accurate sprite0 hit detection
- Scrolling via loopy registers

__APU__
- All channels implemented

__iNES Mappers__ 
- 000 - NROM
- 001 - MMC1B
- 002 - UxROM
- 007 - AxROM
- 094 - UN1ROM
- 180 - Configured UNROM

## Known issues
The DMA timing on APU DMC access is not really implemented

## Functionality verification
CPU OPs are all verified against the good ol' JSON SingleStepTest https://github.com/SingleStepTests/65x02/tree/main/nes6502.
All tests pass and are cycle accurate.

Test roms (https://github.com/christopherpow/nes-test-roms) passing:
* blabla
* blabla
* blabla


## Usage
```bash
$ nes_emulator <rom_path> <flag>
Flags:
       no flag          (regular execution)
       -d | --debug     (debug execution)
       -h | --help      (print this help)
       -v | --validate  (validation execution)
       -v <validation_log_path>  (validate against provided log file)
       -j <path to json test>    (validate CPU against JSON test)
```

## Building from Source

1. Clone the repository and initialize submodules
```bash
git clone https://github.com/babaganosch/nes_emulator.git
cd nes_emulator
git submodule update --recursive --init
```

2. Run the setup script to download and configure build tools, a new shell will be opened with the correct environment variables
```bash
./setup.sh
```

3. Generate build files:
```bash
./generate.sh
```

4. Build the project:
```bash
cd build
make
```

## Project Structure

- `src/` - Source code files
- `include/` - Header files
- `thirdparty/` - Third-party dependencies (git submodules)
- `tools/` - Build tools and utilities
- `data/` - Some logs and roms for validation
- `build/` - Generated build files
- `bin/` - Output directory for compiled binary

## Troubleshooting

### Linux
- I've noticed that the audio thread could be lagging behind on linux producing noise
- If genie complains about being built with a newer GLIBC version, rebuild it locally from source: https://github.com/bkaradzic/GENie/tree/master
- Missing X11 or XKB libraries will cause build failures - install the required development packages
```bash
# Install X11 and XKB development libraries
sudo apt-get install libx11-dev libxkbcommon-dev
```
