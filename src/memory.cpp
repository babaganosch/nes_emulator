#include "nes.hpp"

namespace nes
{

void mem_t::init()
{
    memset(cpu_mem.internal_ram, 0x00, sizeof(cpu_mem.internal_ram));
}

uint8_t* mem_t::memory_read( MEMORY_BUS bus, uint16_t address )
{
    switch (bus)
    {
        case CPU: return cpu_memory_read( address );
        case PPU: return ppu_memory_read( address );
        case APU: return nullptr;
    }
}

void mem_t::memory_write( MEMORY_BUS bus, uint8_t value, uint16_t address )
{
    switch (bus)
    {
        case CPU: cpu_memory_write( value, address ); break;
        case PPU: ppu_memory_write( value, address ); break;
        case APU: break;
    }
}

///////////////////////////// CPU
//////////////////////////////////////////////////////////

uint8_t* mem_t::cpu_memory_read( uint16_t address )
{
    if ( address < 0x2000 )
    { // internal ram
        if ( address < 0x0800 ) return &cpu_mem.internal_ram[ address ];
        else return &cpu_mem.internal_ram[ address % 0x0800 ];
    }

    if ( address < 0x4000 || address == 0x4014 )
    { // ppu registers
        if (address == 0x4014) 
        { // OAMDMA > write
            return nullptr; // TODO: Read latch
        }
        uint16_t reg = 0x2000 + (address % 0x0008);
        switch (reg)
        {
            case( 0x2000 ):
            { // PPUCTRL > write
                return nullptr; // TODO: Read latch
            } break;
            case( 0x2001 ):
            { // PPUMASK > write
                return nullptr; // TODO: Read latch
            } break;
            case( 0x2002 ):
            { // PPUSTATUS > write
                return cpu_mem.ppu_regs.PPUSTATUS;
            } break;
            case( 0x2003 ):
            { // OAMADDR < read
                return nullptr; // TODO: Read latch
            } break;
            case( 0x2004 ):
            { // OAMDATA <> read/write
                return cpu_mem.ppu_regs.OAMDATA;
            } break;
            case( 0x2005 ):
            { // PPUSCROLL >> write x2
                return nullptr; // TODO: Read latch
            } break;
            case( 0x2006 ):
            { // PPUADDR >> write x2
                return nullptr; // TODO: Read latch
            } break;
            case( 0x2007 ):
            { // PPUDATA <> read/write
                return cpu_mem.ppu_regs.PPUDATA;
            } break;
        }
    }

    if ( address < 0x4020 )
    { // apu registers
        return nullptr;
    }

    else
    { // cartridge space
        if ( address < 0x6000 ) return &cartridge_mem.expansion_rom[ address - 0x4020 ];
        if ( address < 0x8000 ) return &cartridge_mem.sram[ address  - 0x6000];
        if ( address < 0xC000 ) return &cartridge_mem.prg_lower_bank[ address - 0x8000 ];
        else return &cartridge_mem.prg_upper_bank[ address - 0xC000 ];
    }
}

void mem_t::cpu_memory_write( uint8_t value, uint16_t address )
{
    if ( address < 0x2000 )
    { // internal ram
        if ( address < 0x0800 ) cpu_mem.internal_ram[ address ] = value;
        else cpu_mem.internal_ram[ address % 0x0800 ] = value;
        return;
    }

    if ( address < 0x4000 || address == 0x4014)
    { // ppu registers
        if (address == 0x4014) 
        { // OAMDMA > write
            *cpu_mem.ppu_regs.OAMDMA = value;
            return;
        }
        uint16_t reg = 0x2000 + (address % 0x0008);
        switch (reg)
        {
            case( 0x2000 ):
            { // PPUCTRL > write
                *cpu_mem.ppu_regs.PPUCTRL = value;
            } break;
            case( 0x2001 ):
            { // PPUMASK > write
                *cpu_mem.ppu_regs.PPUMASK = value;
            } break;
            case( 0x2002 ):
            { // PPUSTATUS > write
                *cpu_mem.ppu_regs.PPUSTATUS = value;
            } break;
            case( 0x2003 ):
            { // OAMADDR < read
                return;
            } break;
            case( 0x2004 ):
            { // OAMDATA <> read/write
                *cpu_mem.ppu_regs.OAMDATA = value;
            } break;
            case( 0x2005 ):
            { // PPUSCROLL >> write x2
                *cpu_mem.ppu_regs.PPUSCROLL = value;
            } break;
            case( 0x2006 ):
            { // PPUADDR >> write x2
                *cpu_mem.ppu_regs.PPUADDR = value;
            } break;
            case( 0x2007 ):
            { // PPUDATA <> read/write
                *cpu_mem.ppu_regs.PPUDATA = value;
            } break;
        }
        return;
    }

    if ( address < 0x4020 )
    { // apu registers
        printf("Failed to write %02X @ $%04X (unmapped APU address space)\n", value, address);
        return;
    }
}

///////////////////////////// PPU
//////////////////////////////////////////////////////////

uint8_t* mem_t::ppu_memory_read( uint16_t address )
{
    return nullptr;
}

void mem_t::ppu_memory_write( uint8_t value, uint16_t address )
{
    
}

} // nes