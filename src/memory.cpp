#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

void mem_t::init( ines_rom_t &rom )
{
    ines_rom = &rom;
    memset(cpu_mem.internal_ram, 0x00, sizeof(cpu_mem.internal_ram));
    for (auto i = 0; i < sizeof(cpu_mem.ram); ++i)
    {
        //cpu_mem.ram[i] = rand() * 0xFF;
    }
}

uint8_t mem_t::memory_read( MEMORY_BUS bus, uint16_t address, bool peek )
{
    switch (bus)
    {
        case CPU: return cpu_memory_read( address, peek );
        case PPU: return ppu_memory_read( address, peek );
        case APU: return 0xFF;
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

uint8_t mem_t::cpu_memory_read( uint16_t address, bool peek )
{
    if ( address < 0x2000 )
    { // internal ram
        return cpu_mem.internal_ram[ address % 0x0800 ];
    }
    
    else if ( address < 0x4000 || address == 0x4014 )
    { // ppu registers
        if (address == 0x4014) 
        { // OAMDMA > write
            return ppu_mem.write_latch;
        }
        
        switch ( address )
        {
            case( 0x2000 ):
            { // PPUCTRL > write
                return ppu_mem.write_latch;
            } break;
            case( 0x2001 ):
            { // PPUMASK > write
                return ppu_mem.write_latch;
            } break;
            case( 0x2002 ):
            { // PPUSTATUS < read
                uint8_t value = ppu->regs.PPUSTATUS;
                if (!peek) {
                    // Clear vblank status bit
                    ppu->regs.PPUSTATUS &= ~0x80;
                    // Reset PPUADDR and write toggle
                    ppu_mem.v.data = 0x0000;
                    ppu_mem.w = 0;
                }
                return value;
            } break;
            case( 0x2003 ):
            { // OAMADDR > write
                return ppu_mem.write_latch;
            } break;
            case( 0x2004 ):
            { // OAMDATA <> read/write
                return ppu_mem.oam.data[ ppu->regs.OAMADDR ];
            } break;
            case( 0x2005 ):
            { // PPUSCROLL >> write x2
                return ppu_mem.write_latch;
            } break;
            case( 0x2006 ):
            { // PPUADDR >> write x2
                return ppu_mem.write_latch;
            } break;
            case( 0x2007 ):
            { // PPUDATA <> read/write
                uint16_t addr = ppu_mem.v.data;
                bool is_palette = (addr >= 0x3F00) && (addr <= 0x3F1F);
                uint8_t data = ppu_mem.ppudata_read_buffer;
                ppu_mem.ppudata_read_buffer = ppu_memory_read( addr, peek );

                // check vram address increment
                if (!is_palette && BIT_CHECK_HI(ppu->regs.PPUCTRL, 2)) 
                {
                    addr+=32;
                }
                else 
                {
                    addr++;
                }
                if (!is_palette)
                {
                    addr = addr % 0x3F00;
                }
                ppu_mem.v.data = addr;
                return data;
            } break;
            case( 0x4014 ):
            { // OAMDMA > write
                return ppu_mem.write_latch;
            }
        }
    }

    else if ( address < 0x4020 )
    { // apu and IO registers
        if ( address == 0x4016 )
        {
            uint8_t value = gamepad[0].latch & 0x1;
            gamepad[0].latch >>= 1;
            return value;
        }
        if ( address == 0x4017 )
        {
            uint8_t value = gamepad[1].latch & 0x1;
            gamepad[1].latch >>= 1;
            return value;
        }
        return 0xFF;
    }

    else
    { // cartridge space
        if ( address < 0x6000 ) return cartridge_mem.expansion_rom[ address - 0x4020 ];
        if ( address < 0x8000 ) return cartridge_mem.sram[ address  - 0x6000];
        if ( address < 0xC000 ) return cartridge_mem.prg_lower_bank[ address - 0x8000 ];
        else return cartridge_mem.prg_upper_bank[ address - 0xC000 ];
    }
    
    return 0x00;
}

void mem_t::cpu_memory_write( uint8_t value, uint16_t address )
{
    if ( address < 0x2000 )
    { // internal ram
        cpu_mem.internal_ram[ address % 0x0800 ] = value;
        return;
    }

    if ( address < 0x4000 || address == 0x4014)
    { // ppu registers

        switch (address)
        {
            case( 0x2000 ):
            { // PPUCTRL > write
                // After power/reset, writes to this register are ignored for about 30,000 cycles.
                if (cpu_cycles < 29658 && ppu->recently_power_on)
                {
                    return;
                }
                // t: ...GH.. ........ <- d: ......GH
                //    <used elsewhere> <- d: ABCDEF..
                ppu_mem.t.nametable = (value & 0b00000011);
                ppu_mem.write_latch = value;
                ppu->regs.PPUCTRL = value;
                ppu->recently_power_on = false;
            } break;
            case( 0x2001 ):
            { // PPUMASK > write
                ppu->regs.PPUMASK = value;
                ppu_mem.write_latch = value;
            } break;
            case( 0x2002 ):
            { // PPUSTATUS < read
                return;
            } break;
            case( 0x2003 ):
            { // OAMADDR < write
                ppu->regs.OAMADDR = value;
                ppu_mem.write_latch = value;
            } break;
            case( 0x2004 ):
            { // OAMDATA <> read/write
                ppu->regs.OAMDATA = value;
                uint8_t addr = ppu->regs.OAMADDR;
                ppu_mem.oam.data[addr++] = value;
                ppu->regs.OAMADDR = addr;
                ppu_mem.write_latch = value;
            } break;
            case( 0x2005 ):
            { // PPUSCROLL >> write x2
                if (ppu_mem.w == 0)
                { // First Write
                    // t: ....... ...ABCDE <- d: ABCDE...
                    // x:              FGH <- d: .....FGH
                    // w:                  <- 1
                    ppu_mem.t.coarse_x = (value & 0b11111000) >> 3;
                    ppu_mem.fine_x = value & 0b111;
                    ppu_mem.w = 1;
                }
                else
                { // Second Write
                    // t: FGH..AB CDE..... <- d: ABCDEFGH
                    // w:                  <- 0
                    ppu_mem.t.coarse_y = (value & 0b11111000) >> 3;
                    ppu_mem.t.fine_y = (value & 0b00000111);
                    ppu_mem.w = 0;
                }
                ppu_mem.write_latch = value;
            } break;
            case( 0x2006 ):
            { // PPUADDR >> write x2
                if (ppu_mem.w == 0)
                { // First Write
                    // t: .CDEFGH ........ <- d: ..CDEFGH
                    //        <unused>     <- d: AB......
                    // t: Z...... ........ <- 0 (bit Z is cleared)
                    // w:                  <- 1
                    ppu_mem.t.data &= 0b000000011111111;
                    ppu_mem.t.data |= (value & 0b00111111) << 8;
                    ppu_mem.w = 1;
                }
                else
                { // Second Write
                    // t: ....... ABCDEFGH <- d: ABCDEFGH
                    // v: <...all bits...> <- t: <...all bits...>
                    // w:                  <- 0
                    ppu_mem.t.data &= 0b111111100000000;
                    ppu_mem.t.data |= value;
                    ppu_mem.v.data = ppu_mem.t.data;
                    ppu_mem.w = 0;
                }
                ppu_mem.write_latch = value;

            } break;
            case( 0x2007 ):
            { // PPUDATA <> read/write
                uint16_t addr = ppu_mem.v.data;
                bool is_palette = (addr >= 0x3F00) && (addr <= 0x3F1F);
                ppu_memory_write( value, addr );

                // check vram address increment
                if (!is_palette && BIT_CHECK_HI(ppu->regs.PPUCTRL, 2))
                {
                    addr+=32;
                }
                else 
                {
                    addr++;
                }
                if (!is_palette)
                {
                    addr = addr % 0x3F00;
                }
                ppu_mem.v.data = addr;
                ppu_mem.write_latch = value;
            } break;
            case ( 0x4014 ):
            { // OAMDMA > write

                // oam addr is 0xXX00 where XX is data
                uint16_t source_addr = (value << 8);
                uint8_t* source = nullptr;
                if ( source_addr < 0x4000 )
                {
                    source = &cpu_mem.internal_ram[ source_addr % 0x0800 ];
                }
                else if (source_addr < 0x6000) 
                {
                    // TODO Fix I/O reg read and mirroring
                    LOG_E("I/O reg read and mirroring not implemented (%04X)", source_addr);
                } 
                else if (source_addr < 0x8000) 
                {
                    // TODO Fix save RAM read
                    LOG_E("Save RAM not implemented (%04X)", source_addr);
                } 
                else if (source_addr < 0xC000) 
                {
                    source = &cartridge_mem.prg_lower_bank[ source_addr - 0x8000 ]; 
                } 
                else 
                {
                    source = &cartridge_mem.prg_upper_bank[ source_addr - 0xC000 ];
                }

                if (!source)
                { // error
                    LOG_E("No source found for OAMDMA write");
                    return;
                }

                // todo take care of cycles here, somehow!
                // The CPU is suspended during the transfer, which will take 513 or 514 cycles after the $4014 write tick.
                // (1 wait state cycle while waiting for writes to complete, +1 if on an odd CPU cycle, then 256 alternating read/write cycles.)
                memcpy( ppu_mem.oam.data, source, 256 );
            } break;
        }
        return;
    }

    if ( address < 0x4020 )
    { // apu and I/O registers

        if ( address == 0x4016 )
        {
            auto old_strobe = gamepad_strobe;
            gamepad_strobe = value & 0x1;
            if (old_strobe == 1 && gamepad_strobe == 0)
            {
                gamepad[0].latch = gamepad[0].data;
                gamepad[1].latch = gamepad[1].data;
            }
            return;
        }

        LOG_W("Failed to write %02X @ $%04X (unmapped APU address space)", value, address);
        return;
    }

}

///////////////////////////// PPU
//////////////////////////////////////////////////////////

uint8_t mem_t::ppu_memory_read( uint16_t address, bool peek )
{
    (void) peek;
    if ( address < 0x2000 )
    { // patterntables
        return cartridge_mem.chr_rom[ address ];
    }

    else if ( address < 0x3F00 )
    { // nametables
        uint16_t t_addr = address - 0x2000;
        switch( ppu_mem.nt_mirroring )
        {
            case( ppu_mem_t::nametable_mirroring::horizontal ):
            {
                t_addr %= 0x1000;
                if ( t_addr >= 0x0800 )
                { // B
                    if ( t_addr >= 0x0C00 )
                    { // Mirror B
                        t_addr -= 0x0400;
                    }
                    t_addr -= 0x0400;
                } 
                else if ( t_addr >= 0x0400 )
                { // Mirror A
                    t_addr -= 0x0400;
                }
            } break;
            case( ppu_mem_t::nametable_mirroring::vertical ):
            {
                t_addr %= 0x800;
            } break;
            case( ppu_mem_t::nametable_mirroring::single_screen ):
            {
                t_addr %= 0x400;
            } break;
            default:
            {
                LOG_E("Unimplemented nametable mirroring mode: %d", ppu_mem.nt_mirroring );
            } break;
        }
        address = t_addr;
        return ppu_mem.vram[ address ];
    }

    else if ( address < 0x3FFF )
    { // palettes
        uint16_t wrapped_addr = (address - 0x3F00) % 0x20;
        if (wrapped_addr == 0x10 ||
            wrapped_addr == 0x14 ||
            wrapped_addr == 0x18 ||
            wrapped_addr == 0x1C)
        {
            wrapped_addr = wrapped_addr - 0x10;
        }

        return ppu_mem.palette[ wrapped_addr ];
    }
    
    LOG_E("PPU memory read on weird address (%04X)", address );
    throw RESULT_ERROR;
}

void mem_t::ppu_memory_write( uint8_t value, uint16_t address )
{ // Writing to VRAM
    if ( address < 0x2000 )
    { // pattern tables
        cartridge_mem.chr_rom[ address ] = value;
        return;
    }

    else if (address < 0x3F00) 
    { // nametables
        uint16_t t_addr = address - 0x2000;
        switch( ppu_mem.nt_mirroring )
        {
            case( ppu_mem_t::nametable_mirroring::horizontal ):
            {
                t_addr %= 0x1000;
                if ( t_addr >= 0x0800 )
                { // B
                    if ( t_addr >= 0x0C00 )
                    { // Mirror B
                        t_addr -= 0x0400;
                    }
                    t_addr -= 0x0400;
                } 
                else if ( t_addr >= 0x0400 )
                { // Mirror A
                    t_addr -= 0x0400;
                }
            } break;
            case( ppu_mem_t::nametable_mirroring::vertical ):
            {
                t_addr %= 0x800;
            } break;
            case( ppu_mem_t::nametable_mirroring::single_screen ):
            {
                t_addr %= 0x400;
            } break;
            default:
            {
                LOG_E("Unimplemented nametable mirroring mode: %d", ppu_mem.nt_mirroring );
            } break;
        }
        address = t_addr;
        ppu_mem.vram[ address ] = value;
        return;
    } 

    else if ( address < 0x3FFF )
    { // palettes
        uint16_t wrapped_addr = (address - 0x3F00) % 0x20;
        if (wrapped_addr == 0x10 ||
            wrapped_addr == 0x14 ||
            wrapped_addr == 0x18 ||
            wrapped_addr == 0x1C)
        {
            wrapped_addr = wrapped_addr - 0x10;
        }

        ppu_mem.palette[ wrapped_addr ] = value;
        return;
    }

    LOG_E("PPU memory write on weird address (%04X)", address );
    throw RESULT_ERROR;
}

} // nes