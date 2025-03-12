# NesScape

A Nintendo Entertainment System (NES) emulator implementation in C++ that aims to provide cycle accurate emulation of the original NTSC NES hardware. Implemented and tested on MacOS, but should _probably_ work on Windows and Linux as well.


https://github.com/user-attachments/assets/c4a952a7-8922-4ecf-bc93-74d56e77b573

<details>
<summary>More videos</summary>

https://github.com/user-attachments/assets/378fbb82-612a-4daa-9953-0f6c1763a9b5

https://github.com/user-attachments/assets/cb087da7-b4c0-493b-ab79-62cf1c3a512e

https://github.com/user-attachments/assets/220112f3-10ca-46bf-9e04-d2785a2aaab9

https://github.com/user-attachments/assets/9e4b713a-1cbe-49af-a214-512b8a22bf71

</details>


## Features

__CPU__
- All 256 opcodes implemented and cycle accurate
- BRK, IRQ and NMI implemented with vector hijacking

__PPU__
- Accurate pixel rendering
- Accurate sprite0 hit detection
- Scrolling via loopy registers

__APU__
- Pulse channels implemented
- Triangle channels implemented
- Noise channel implemented
- DMC channel _mostly_ implemented

__iNES Mappers__ 
- 000 - NROM
- 001 - MMC1B
- 002 - UxROM
- 007 - AxROM
- 094 - UN1ROM
- 180 - Configured UNROM

## Functionality verification
CPU OPs are all verified against the good ol' JSON SingleStepTest https://github.com/SingleStepTests/65x02/tree/main/nes6502.
All tests pass and are cycle accurate.

Test roms (https://github.com/christopherpow/nes-test-roms) passing:  
- CPU / Memory
    - nes_test - __23 / 23__
    - branch_timing_tests - __3 / 3__
    - blargg_nes_cpu_test5 - official - __11 / 11__
    - blargg_nes_cpu_test5 - nes - __10 / 11__
    - cpu_exec_space - __1 / 2__
    - cpu_interrupts_v2 - __3 / 5__
    - cpu_timing_test6 - __3 / 3__
    - instr_misc - __2 / 4__
    - instr_test-v5 - __15 / 16__
    - instr_timing - __2 / 2__
    - nes_instr_test - __10 / 11__
    - oam_read - __1 / 1__
    - oam_stress - __0 / 1__
    
- PPU
    - ppu_vbl_nmi - __10 / 10__
    - vbl_nmi_timing - __7 / 7__
    - scanline - __1 / 1__
    - sprite_hit_tests_2005.10.05 - __11 / 11__
    - sprite_overflow_tests - __0 / 5__

- APU
    - apu_test - __7 / 8__
    - blargg_apu_2005.07.30 - __10 / 11__
    - dmc_tests - __4 / 4__

__Note:__ Sometimes the full test fails but all the singles succeed.

## Known issues / limitations
I have not implemented any type of save state yet. Neither have I implemented any kind of RESET functionality. Sprite overflow is not implemented yet.
The DMA timing on APU DMC access is not really implemented. On Battletoads, text and logos on the title screen is a little weird, and audio seems a little off when playing the game.

## Usage
After compiling, the binary will end up in the `bin/` directory.
```bash
$ nesscape <rom_path> <flag>
Flags:
       no flag          (regular execution)
       -d | --debug     (debug execution)
       -h | --help      (print this help)
       -v | --validate  (validation execution)
       -v <validation_log_path>  (validate against provided log file)
       -j <path to json test>    (validate CPU against JSON test)
```

## Compiling

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
- `tools/` - Build tools ([GENie](https://github.com/bkaradzic/GENie/tree/master) and [Ninja](https://ninja-build.org/))
- `data/` - Some old validation logs for nestest
- `build/` - Generated build files
- `bin/` - Output directory for compiled binary

## Dependecies / Submodules

[MiniAudio](https://miniaud.io/) - Single source audio playback and capture library for C and C++.

[MiniFB](https://github.com/emoon/minifb) - Small cross platform library that makes it easy to render pixels in a window.

[RapidJSON](https://rapidjson.org/) - A fast JSON parser/generator for C++ with both SAX/DOM style API. (Used only for parsing JSON tests for validation)

[NesTestRoms](https://github.com/christopherpow/nes-test-roms) - A collection of test roms used for functionality validation.

## Troubleshooting

__Windows__  
I've only tested the emulator briefly on windows, but it seems to work fine if compiled with gcc.

__Linux__  
Just like windows, I've only tested the emulator very briefly on Ubuntu. Here's some small details I've noticed..
- The audio thread does not sync very well and starts lagging behind, producing a disturbing noise.
- If genie complains about being built with a newer GLIBC version, rebuild it locally from source: https://github.com/bkaradzic/GENie/tree/master
- Missing X11 or XKB libraries will cause build failures - install the required development packages
```bash
# Install X11 and XKB development libraries
sudo apt-get install libx11-dev libxkbcommon-dev
```

## Who am I ?
[larsandersson.info](https://larsandersson.info)
