#include "mappers.hpp"
#include "logging.hpp"
#include <memory>

namespace nes
{

mapper_t* mappers_lut[256] = {
    new mapper_nrom_t(),        // 0
    nullptr,                    // 1
    new mapper_uxrom_t(),       // 2
    nullptr,                    // 3
    nullptr,                    // 4
    // ...
};

//////// mapper basic behaviour
void mapper_t::init( mem_t* memory_ref ) {
    memory = memory_ref;
    uint8_t prg_banks = memory->ines_rom->header.prg_size;
    uint8_t chr_banks = memory->ines_rom->header.chr_size;

    LOG_D("PRG Banks: %u", prg_banks);
    LOG_D("CHR Banks: %u", chr_banks);

    // Map PRG ROM
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[0];
    memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[prg_banks-1];
    // Map CHR ROM/RAM
    if (chr_banks > 0) 
    { // ROM
        memory->cartridge_mem.chr_rom = memory->ines_rom->chr_pages[0];
    } else
    { // RAM
        memory->cartridge_mem.chr_mode = &memory->cartridge_mem.chr_ram;
        memory->cartridge_mem.chr_ram = new uint8_t[CHR_PAGE_SIZE];
    }
}

uint8_t mapper_t::cpu_read( uint16_t address ) {
    if ( address < 0x6000 ) return memory->cartridge_mem.expansion_rom[ address - 0x4020 ];
    if ( address < 0x8000 ) return memory->cartridge_mem.sram[ address  - 0x6000];
    if ( address < 0xC000 ) return memory->cartridge_mem.prg_lower_bank[ address - 0x8000 ];
    else return memory->cartridge_mem.prg_upper_bank[ address - 0xC000 ];
}

uint8_t mapper_t::ppu_read( uint16_t address ) {
    return memory->cartridge_mem.chr_rom[ address ];
}

void mapper_t::cpu_write( uint16_t address, uint8_t value ) {
    return;
}

void mapper_t::ppu_write( uint16_t address, uint8_t value ) {
    memory->cartridge_mem.chr_rom[ address ] = value;
}

//////// mapper 000 - NROM


//////// mapper 002 - UxROM
uint8_t mapper_uxrom_t::ppu_read( uint16_t address ) {
    return memory->cartridge_mem.chr_ram[ address ];
}

void mapper_uxrom_t::cpu_write( uint16_t address, uint8_t value ) {
    uint8_t bank = address & 0b00000111;
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[bank];
}

void mapper_uxrom_t::ppu_write( uint16_t address, uint8_t value ) {
    memory->cartridge_mem.chr_ram[ address ] = value;
}


} // nes