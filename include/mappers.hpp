#ifndef MAPPERS_HPP
#define MAPPERS_HPP

#include "nes.hpp"
#include <cstdint>

namespace nes
{

constexpr uint32_t CHR_8KB_SIZE = 8 * 1024;
constexpr uint32_t CHR_4KB_SIZE = 4 * 1024;

extern mapper_t* mappers_lut[256];
void instantiate_mappers();
/*
*   NOTE: Instantiate each implemented mapper in the emu_t::init()
*/

//////// mapper 000 - NROM
struct mapper_nrom_t : public mapper_t { };

//////// mapper 001 - MMC1B
struct mapper_mmc1b_t : public mapper_t {
    void cpu_write( uint16_t address, uint8_t value ) override;

    uint8_t prg_bank_mode{3};
    uint8_t chr_bank_mode{0};

    uint8_t write{0x0};
    uint16_t sr{0b10000};
    uint8_t pb{0x0};
};

//////// mapper 002 - UxROM (UNROM & UOROM)
struct mapper_uxrom_t : public mapper_t {
    void cpu_write( uint16_t address, uint8_t value ) override;
};

//////// mapper 007 - AxROM
struct mapper_axrom_t : public mapper_t {
    void init( mem_t* memory_ref ) override;
    void cpu_write( uint16_t address, uint8_t value ) override;
};

//////// mapper 094 - UN1ROM
struct mapper_un1rom_t : public mapper_t {
    void cpu_write( uint16_t address, uint8_t value ) override;
};

//////// mapper 180 - UNROM (configured bank select)
struct mapper_unrom_configured_t : public mapper_t {
    void init( mem_t* memory_ref ) override;
    void cpu_write( uint16_t address, uint8_t value ) override;
};
    
} // nes

#endif /* MAPPERS_HPP */