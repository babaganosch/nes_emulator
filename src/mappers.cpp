#include "mappers.hpp"
#include "logging.hpp"
#include <memory>

namespace nes
{

mapper_t* mappers_lut[256] = { nullptr };

void instantiate_mappers()
{
    mappers_lut[0]   = new mapper_nrom_t();
    mappers_lut[1]   = new mapper_mmc1b_t();
    mappers_lut[2]   = new mapper_uxrom_t();
    mappers_lut[7]   = new mapper_axrom_t();
    mappers_lut[94]  = new mapper_un1rom_t();
    mappers_lut[180] = new mapper_unrom_configured_t();

    uint8_t mappers_instantiated = 0;
    for (uint16_t i = 0; i < 256; ++i) {
        if (mappers_lut[i]) ++mappers_instantiated;
    }
    LOG_I("%u mappers instantiated", mappers_instantiated);
}

//////// mapper basic behaviour
void mapper_t::init( mem_t* memory_ref ) {
    memory = memory_ref;
    uint8_t prg_banks = memory->ines_rom->header.prg_size;
    uint8_t chr_banks = memory->ines_rom->header.chr_size;

    LOG_D("PRG Banks: %u (%u KB)", prg_banks, prg_banks*16);
    LOG_D("CHR Banks: %u (%u KB)", chr_banks, chr_banks*8);

    // Map PRG ROM
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[0];
    memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[prg_banks-1];

    // Map CHR ROM
    if (chr_banks > 0) 
    { // Cartridge contains CHR ROM
        memcpy(memory->cartridge_mem.chr_rom.chr_bank_8kb, memory->ines_rom->chr_pages[0], CHR_8KB_SIZE);
    }
}

uint8_t mapper_t::cpu_read( uint16_t address ) {
    if ( address < 0x6000 ) return memory->cartridge_mem.expansion_rom[ address - 0x4020 ];
    if ( address < 0x8000 ) return memory->cartridge_mem.sram[ address  - 0x6000 ];
    if ( address < 0xC000 ) return memory->cartridge_mem.prg_lower_bank[ address - 0x8000 ];
    else return memory->cartridge_mem.prg_upper_bank[ address - 0xC000 ];
}

uint8_t mapper_t::ppu_read( uint16_t address ) {
    return memory->cartridge_mem.chr_rom.chr_bank_8kb[ address ];
}

void mapper_t::cpu_write( uint16_t address, uint8_t value ) {
    if ( address >= 0x6000 && address < 0x8000 )
    { // SRAM $6000 - $7FFF
        memory->cartridge_mem.sram[ address - 0x6000 ] = value;
    }
}

void mapper_t::ppu_write( uint16_t address, uint8_t value ) {
    memory->cartridge_mem.chr_rom.chr_bank_8kb[ address ] = value;
}

//////// mapper 001 - MMC1B
void mapper_mmc1b_t::cpu_write( uint16_t address, uint8_t value ) {
    mapper_t::cpu_write( address, value );

    if ( address < 0x8000 ) return;
    if ( value & 0x80 )
    { // Clear
        sr = 0b10000;
        write = 0;
        memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[memory->ines_rom->header.prg_size - 1];
        prg_bank_mode = 3;
    } else 
    { // Shift
        sr >>= 1;
        sr |= (value & 0x1) << 4;

        if (++write >= 5) {
            pb = sr & 0b11111;
            sr = 0b10000;
            write = 0;

            if ( address < 0xA000 ) 
            { // CONTROL
                switch (pb & 0b11)
                {
                    case 0: {
                        memory->ppu_mem.nt_mirroring = ppu_mem_t::nametable_mirroring::single_screen_lower;
                    } break;
                    case 1: {
                        memory->ppu_mem.nt_mirroring = ppu_mem_t::nametable_mirroring::single_screen_higher;
                    } break;
                    case 2: {
                        memory->ppu_mem.nt_mirroring = ppu_mem_t::nametable_mirroring::vertical;
                    } break;
                    case 3: {
                        memory->ppu_mem.nt_mirroring = ppu_mem_t::nametable_mirroring::horizontal;
                    } break;
                }
                prg_bank_mode = (pb & 0b01100) >> 2;
                chr_bank_mode = (pb & 0b10000) >> 4;
            } else if ( address < 0xC000 )
            { // CHR Bank 0
                uint32_t size = chr_bank_mode == 0 ? CHR_8KB_SIZE : CHR_4KB_SIZE;
                uint32_t  offset = chr_bank_mode == 0 ? 0 : (pb & 0b1 ? CHR_4KB_SIZE : 0);
                uint8_t   bank = (pb & 0b11110) >> 1;
                uint8_t** src = memory->ines_rom->chr_pages;
                if (memory->ines_rom->header.chr_size == 0) 
                { // CHR RAM
                    src = memory->ines_rom->prg_pages;
                }
                memcpy(memory->cartridge_mem.chr_rom.chr_bank_8kb, src[bank] + offset, size);
                
            } else if ( address < 0xE000 )
            { // CHR Bank 1
                uint8_t** src = memory->ines_rom->chr_pages;
                uint32_t  offset = chr_bank_mode == 0 ? 0 : (pb & 0b1 ? CHR_4KB_SIZE : 0);
                uint8_t   bank = (pb & 0b11110) >> 1;
                if (memory->ines_rom->header.chr_size == 0) 
                { // CHR RAM
                    src = memory->ines_rom->prg_pages;
                }
                memcpy(memory->cartridge_mem.chr_rom.chr_bank_4kb_upper, src[bank] + offset, CHR_4KB_SIZE);
            } else
            { // PRG bank
                switch (prg_bank_mode)
                {
                    case 0:
                    case 1:
                    { // 32KB mode
                        uint8_t bank = (pb & 0b1110) >> 1;
                        memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[bank];
                        memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[bank+1];
                    } break;
                    case 2: 
                    { // 16KB mode - fixed first bank at low bank
                        memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[0];
                        memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[pb & 0b1111];
                    } break;
                    case 3: 
                    { // 16KB mode - fixed last bank at high bank
                        memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[pb & 0b1111];
                        memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[memory->ines_rom->header.prg_size - 1];
                    } break;
                }
            }
        }   
    }
}


//////// mapper 002 - UxROM
void mapper_uxrom_t::cpu_write( uint16_t address, uint8_t value ) {
    uint8_t bank = value & 0b00000111;
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[bank];
}

void mapper_mmc3_t::cpu_write( uint16_t address, uint8_t value ) {
    // TODO
}

//////// mapper 007 - AxROM
void mapper_axrom_t::init( mem_t* memory_ref ) {
    mapper_t::init( memory_ref );
    // ReMap PRG banks
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[0];
    memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[1];
}

void mapper_axrom_t::cpu_write( uint16_t address, uint8_t value ) {
    uint8_t prg_bank = (value & 0b00000111);
    uint8_t vram_page = (value & 0b00010000) >> 4;
    memory->ppu_mem.nt_mirroring = (ppu_mem_t::nametable_mirroring)(2 + vram_page);
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[(prg_bank*2)];
    memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[(prg_bank*2)+1];
}

//////// mapper 094 - UN1ROM
void mapper_un1rom_t::cpu_write( uint16_t address, uint8_t value ) {
    uint8_t bank = (value & 0b00011100) >> 2;
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[bank];
}

//////// mapper 180 - Configured UNROM
void mapper_unrom_configured_t::init( mem_t* memory_ref ) {
    mapper_t::init( memory_ref );
    // ReMap PRG banks
    uint8_t prg_banks = memory->ines_rom->header.prg_size;
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[prg_banks-1];
    memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[0];
}

void mapper_unrom_configured_t::cpu_write( uint16_t address, uint8_t value ) {
    uint8_t bank = value & 0b00000111;
    memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[bank];
}

} // nes