#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

void mem_t::init()
{
    memset(cpu_mem.internal_ram, 0x00, sizeof(cpu_mem.internal_ram));
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
            return ppu->write_latch;
        }

        uint16_t reg = 0x2000 + ((address - 0x2000) % 0x0008);
        
        switch (reg)
        {
            case( 0x2000 ):
            { // PPUCTRL > write
                return ppu->write_latch;
            } break;
            case( 0x2001 ):
            { // PPUMASK > write
                return ppu->write_latch;
            } break;
            case( 0x2002 ):
            {
                /* PPUSTATUS < read
                 *
                 * 7  bit  0
                 * ---- ----
                 * VSO. ....
                 * |||| ||||
                 * |||+-++++- PPU open bus. Returns stale PPU bus contents.
                 * ||+------- Sprite overflow. The intent was for this flag to be set
                 * ||         whenever more than eight sprites appear on a scanline, but a
                 * ||         hardware bug causes the actual behavior to be more complicated
                 * ||         and generate false positives as well as false negatives; see
                 * ||         PPU sprite evaluation. This flag is set during sprite
                 * ||         evaluation and cleared at dot 1 (the second dot) of the
                 * ||         pre-render line.
                 * |+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
                 * |          a nonzero background pixel; cleared at dot 1 of the pre-render
                 * |          line.  Used for raster timing.
                 * +--------- Vertical blank has started (0: not in vblank; 1: in vblank).
                 *         Set at dot 1 of line 241 (the line *after* the post-render
                 *         line); cleared after reading $2002 and at dot 1 of the
                 *         pre-render line.
                 */
                uint8_t value = ppu->regs.PPUSTATUS;
                if (!peek) {
                    ppu->regs.PPUSTATUS &= ~0x80;   // clear vblank status bit if register is being read
                    ppu->regs.PPUADDR[0] = 0x00;    // PPUADDR is also zeroed
                    ppu->regs.PPUADDR[1] = 0x00;
                }
                return value;
            } break;
            case( 0x2003 ):
            { // OAMADDR > write
                return ppu->write_latch;
            } break;
            case( 0x2004 ):
            { // OAMDATA <> read/write
                return ppu_mem.oam[ ppu->regs.OAMADDR ];
            } break;
            case( 0x2005 ):
            { // PPUSCROLL >> write x2
                return ppu->write_latch;
            } break;
            case( 0x2006 ):
            { // PPUADDR >> write x2
                return ppu->write_latch;
            } break;
            case( 0x2007 ):
            { // PPUDATA <> read/write
                uint16_t addr = ppu->regs.PPUADDR[0] << 8 | ppu->regs.PPUADDR[1];
                bool is_palette = addr >= 0x3F00 && addr <= 0x3F1F;
                uint8_t data = ppu_memory_read( addr, peek );

                // check vram address increment
                if (!is_palette && ((ppu->regs.PPUCTRL >> 2) == 0x1)) 
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
                ppu->regs.PPUADDR[0] = (addr & 0xFF00) >> 8;
                ppu->regs.PPUADDR[1] = (addr & 0x00FF);
                return data;
            } break;
        }
    }

    else if ( address < 0x4020 )
    { // apu and IO registers
        return 0x00;
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
        if (address == 0x4014) 
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
            } 
            else if (source_addr < 0x8000) 
            {
                // TODO Fix save RAM read
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
                return;
            }

            // todo take care of cycles here, somehow!
            // The CPU is suspended during the transfer, which will take 513 or 514 cycles after the $4014 write tick.
            // (1 wait state cycle while waiting for writes to complete, +1 if on an odd CPU cycle, then 256 alternating read/write cycles.)
            memcpy( &ppu_mem.oam[ppu->regs.OAMADDR], source, 256 );//  &ppu.oam[ppu.oamaddr], dma_data_ptr, 256);

            return;
        }

        uint16_t reg = 0x2000 + ((address - 0x2000) % 0x0008);
        switch (reg)
        {
            case( 0x2000 ):
            {
                /* PPUCTRL > write
                 *
                 *  7  bit  0
                 *  ---- ----
                 *  VPHB SINN
                 *  |||| ||||
                 *  |||| ||++- Base nametable address
                 *  |||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
                 *  |||| |+--- VRAM address increment per CPU read/write of PPUDATA
                 *  |||| |     (0: add 1, going across; 1: add 32, going down)
                 *  |||| +---- Sprite pattern table address for 8x8 sprites
                 *  ||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
                 *  |||+------ Background pattern table address (0: $0000; 1: $1000)
                 *  ||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM#Byte 1)
                 *  |+-------- PPU master/slave select
                 *  |          (0: read backdrop from EXT pins; 1: output color on EXT pins)
                 *  +--------- Generate an NMI at the start of the
                 *             vertical blanking interval (0: off; 1: on)
                 */
                
                // After power/reset, writes to this register are ignored for about 30,000 cycles.
                if (cpu_cycles < 29658 && ppu->recently_power_on)
                {
                    ppu->recently_power_on = false;
                    return;
                }
                ppu->regs.PPUCTRL = value;
                ppu->write_latch = value;
            } break;
            case( 0x2001 ):
            {
                /* PPUMASK > write
                 *
                 * 7  bit  0
                 * ---- ----
                 * BGRs bMmG
                 * |||| ||||
                 * |||| |||+- Greyscale (0: normal color, 1: produce a greyscale display)
                 * |||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
                 * |||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
                 * |||| +---- 1: Show background
                 * |||+------ 1: Show sprites
                 * ||+------- Emphasize red (green on PAL/Dendy)
                 * |+-------- Emphasize green (red on PAL/Dendy)
                 * +--------- Emphasize blue
                 */
                ppu->regs.PPUMASK = value;
                ppu->write_latch = value;
            } break;
            case( 0x2002 ):
            { // PPUSTATUS < read
                return;
            } break;
            case( 0x2003 ):
            { // OAMADDR < write
                ppu->regs.OAMADDR = value;
                ppu->write_latch = value;
            } break;
            case( 0x2004 ):
            { // OAMDATA <> read/write
                ppu->regs.OAMDATA = value;
                uint8_t addr = ppu->regs.OAMADDR;
                ppu_mem.oam[addr++] = value;
                ppu->regs.OAMADDR = addr;
                ppu->write_latch = value;
            } break;
            case( 0x2005 ):
            { // PPUSCROLL >> write x2
                ppu->regs.PPUSCROLL[ppu_mem.ppuscroll_y_byte_flag] = value;
                ppu_mem.ppuscroll_y_byte_flag = !ppu_mem.ppuscroll_y_byte_flag;
                ppu->write_latch = value;
            } break;
            case( 0x2006 ):
            { // PPUADDR >> write x2
                if (ppu_mem.ppuaddr_lo_byte_flag == 0)
                { // Valid addresses are $0000–$3FFF; higher addresses will be mirrored down.
                    value = value % 0x40;
                }
                ppu->regs.PPUADDR[ppu_mem.ppuaddr_lo_byte_flag] = value;
                ppu_mem.ppuaddr_lo_byte_flag = !ppu_mem.ppuaddr_lo_byte_flag;
                ppu->write_latch = value;
            } break;
            case( 0x2007 ):
            { // PPUDATA <> read/write
                uint16_t addr = (ppu->regs.PPUADDR[0] << 8) | ppu->regs.PPUADDR[1];
                bool is_palette = addr >= 0x3F00 && addr <= 0x3F1F;
                ppu_memory_write( value, addr );

                // check vram address increment
                if (!is_palette && ((ppu->regs.PPUCTRL >> 2) & 0x1)) 
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
                ppu->regs.PPUADDR[0] = (addr & 0xFF00) >> 8;
                ppu->regs.PPUADDR[1] = (addr & 0x00FF);
                ppu->regs.PPUDATA = value;
                ppu->write_latch = value;
            } break;
        }
        return;
    }

    if ( address < 0x4020 )
    { // apu registers
        LOG_W("Failed to write %02X @ $%04X (unmapped APU address space)", value, address);
        return;
    }

    // TODO, implement writing to sram
}

///////////////////////////// PPU
//////////////////////////////////////////////////////////

uint8_t mem_t::ppu_memory_read( uint16_t address, bool peek )
{
    (void) peek;
    if ( address < 0x2000 )
    { // patterntables
        return ppu->memory->cartridge_mem.chr_rom[ address ];
    }

    else if ( address < 0x3F00 )
    { // nametables
        uint16_t t_addr = address - 0x2000;
        if (address < 0x2800) {
            t_addr = t_addr % 0x400;
        } else {
            t_addr = (t_addr - 0x800) % 0x400;
            t_addr = t_addr + 0x400;
        }

        uint16_t wrapped_addr = t_addr % 0x800;

        return ppu_mem.vram[wrapped_addr];
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
    
    return 0xFF;
}

void mem_t::ppu_memory_write( uint8_t value, uint16_t address )
{ // Writing to VRAM
    if ( address < 0x2000 )
    { // patterntables
        // ignore for now
    }

    else if (address < 0x3F00) 
    { // nametables
        uint16_t t_addr = address - 0x2000;
        if (address < 0x2800) {
            t_addr = t_addr % 0x400;
        } else {
            t_addr = (t_addr - 0x800) % 0x400;
            t_addr = t_addr + 0x400;
        }

        uint16_t wrapped_addr = t_addr % 0x800;

        ppu_mem.vram[wrapped_addr] = value;
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
    }
}

} // nes