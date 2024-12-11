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
    uint8_t ppu_read( uint16_t address ) override;
    void cpu_write( uint16_t address, uint8_t value ) override;
    void ppu_write( uint16_t address, uint8_t value ) override;
};

extern mapper_t* mappers_lut[256];
    
} // nes

#endif /* MAPPERS_HPP */