#include "nes.hpp"

namespace nes
{

namespace
{
const uint16_t ram_mirror_offset = 0x0800;
const uint16_t ppu_offset = 0x2000;
//const uint16_t ppu_mirror_offset = 0x0008;
const uint16_t apu_offset = 0x4000;
const uint16_t cartridge_offset = 0x4020;
const uint16_t cartridge_sram_offset = 0x6000;
const uint16_t cartridge_prg_lower_offset = 0x8000;
const uint16_t cartridge_prg_upper_offset = 0xC000;
} // anonymous

void mem_t::init()
{
    memset(cpu_mem.internal_ram, 0x00, sizeof(cpu_mem.internal_ram));
}

uint8_t* mem_t::memory_read( uint16_t address )
{
    if ( address < ppu_offset )
    { // cpu_mem_t
        if ( address < ram_mirror_offset ) return &cpu_mem.internal_ram[ address ];
        else return &cpu_mem.internal_ram[ address % ram_mirror_offset ];
    }

    if ( address < apu_offset )
    { // ppu_mem_t
        if ( address < ppu_offset ) return nullptr;
        else return nullptr; // [ ppu_offset + (address % ppu_mirror_offset) ]
    }

    if ( address < cartridge_offset )
    { // apu_mem_t
        return nullptr;
    }

    else
    { // cartridge_mem_t
        if ( address < cartridge_sram_offset ) return &cartridge_mem.expansion_rom[ address - cartridge_offset ];
        if ( address < cartridge_prg_lower_offset ) return &cartridge_mem.sram[ address  - cartridge_sram_offset];
        if ( address < cartridge_prg_upper_offset ) return &cartridge_mem.prg_lower_bank[ address - cartridge_prg_lower_offset ];
        else return &cartridge_mem.prg_upper_bank[ address - cartridge_prg_upper_offset ];
    }
}

void mem_t::memory_write( uint8_t value, uint16_t address )
{
    if ( address < ppu_offset )
    { // cpu_mem_t
        if ( address < ram_mirror_offset ) cpu_mem.internal_ram[ address ] = value;
        else cpu_mem.internal_ram[ address % ram_mirror_offset ] = value;
    }

    if ( address < apu_offset )
    { // ppu_mem_t
        if ( address < ppu_offset ) ;
        else ; // [ ppu_offset + (address % ppu_mirror_offset) ]
        printf("Failed to write %02X @ $%04x (unmapped PPU address space)\n", value, address);
    }

    if ( address < cartridge_offset )
    { // apu_mem_t
        printf("Failed to write %02X @ $%04x (unmapped APU address space)\n", value, address);
    }

    else
    { // cartridge_mem_t
        if ( address < cartridge_sram_offset ) cartridge_mem.expansion_rom[ address - cartridge_offset] = value;
        if ( address < cartridge_prg_lower_offset ) cartridge_mem.sram[ address - cartridge_sram_offset] = value;
        if ( address < cartridge_prg_upper_offset ) cartridge_mem.prg_lower_bank[ address - cartridge_prg_lower_offset ] = value;
        else cartridge_mem.prg_upper_bank[ address - cartridge_prg_upper_offset ] = value;
    }
}

} // nes