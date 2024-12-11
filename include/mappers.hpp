#ifndef MAPPERS_HPP
#define MAPPERS_HPP

#include "logging.hpp"
#include "nes.hpp"
#include <cstdint>

namespace nes
{

//////// mapper 000 - NROM
struct mapper_nrom_t : public mapper_t { };

//////// mapper 001 - MMC1B

//////// mapper 002 - UxROM
struct mapper_uxrom_t : public mapper_t {
    void cpu_write( uint16_t address, uint8_t value ) override;
};

//////// mapper 094 - UN1ROM
struct mapper_un1rom_t : public mapper_t {
    void cpu_write( uint16_t address, uint8_t value ) override;
};

//////// mapper 180 - UNROM
struct mapper_unrom_t : public mapper_t {
    void init( mem_t* memory_ref ) override;
    void cpu_write( uint16_t address, uint8_t value ) override;
};

extern mapper_t* mappers_lut[256];
    
} // nes

#endif /* MAPPERS_HPP */