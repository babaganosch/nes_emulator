#include "nes.hpp"
#include "logging.hpp"
#include "mappers.hpp"
#include <memory>

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

    // Mapper (0-255 only)
    uint8_t mapper_identifier = ((ines_rom->header.flags_7 & 0xF0) | ((ines_rom->header.flags_6 & 0xF0) >> 4) % 256);
    mapper = mappers_lut[mapper_identifier];
    LOG_D("Mapper: %u", mapper_identifier);
    if (!mapper) {
        LOG_E("Mapper unimplemented");
        throw RESULT_ERROR;
    }

    // Map PRG ROM and CHR ROM/RAM
    mapper->init( this );

    // Mirroring
    if (BIT_CHECK_HI(ines_rom->header.flags_6, 0))
    {
        ppu_mem.nt_mirroring = ppu_mem_t::nametable_mirroring::vertical;
        LOG_D("Vertical mirroring (horizontal arrangement)");
    } else {
        LOG_D("Horizontal mirroring (vertical arrangement)");
    }

    // Persistent memory on cartridge
    if (BIT_CHECK_HI(ines_rom->header.flags_6, 1))
    {
        LOG_W("Cartridge contains some kind of persistent memory (unimplemented)");
    }

    LOG_I("Memory layout initiated successfully");
}

uint8_t mem_t::memory_read( MEMORY_BUS bus, uint16_t address, bool peek )
{
    uint8_t data = 0xFF;
    switch (bus)
    {
        case CPU: data = cpu_memory_read( address, peek ); break;
        case PPU: data = ppu_memory_read( address, peek ); break;
        case APU: break;
    }
    cpu_mem.activity.address = address;
    cpu_mem.activity.value = data;
    cpu_mem.activity.read = true;
    return data;
}

void mem_t::memory_write( MEMORY_BUS bus, uint8_t value, uint16_t address )
{
    switch (bus)
    {
        case CPU: cpu_memory_write( value, address ); break;
        case PPU: ppu_memory_write( value, address ); break;
        case APU: break;
    }
    cpu_mem.activity.address = address;
    cpu_mem.activity.value = value;
    cpu_mem.activity.read = false;
}

///////////////////////////// CPU
//////////////////////////////////////////////////////////

uint8_t* mem_t::fetch_byte_ref( uint16_t address )
{
    uint8_t* ref = nullptr;
    if ( address < 0x2000 )
    { // internal ram
        ref = &cpu_mem.internal_ram[ address ];
    }
    else if ( address < 0x4020 )
    {
        LOG_E("Trying to fetch unmapped reference (%04x)", address);
    }
    else if ( address < 0x6000 )
    { // expansion rom
        ref = &cartridge_mem.expansion_rom[ address - 0x4020 ];
    }
    else if ( address < 0x8000 )
    { // sram
        ref = &cartridge_mem.sram[ address - 0x6000 ];
    }
    else if ( address < 0xC000 )
    { // prg lower
        ref = &cartridge_mem.prg_lower_bank[ address - 0x8000 ];
    }
    else if ( address <= 0xFFFF )
    { // prg upper
        ref = &cartridge_mem.prg_upper_bank[ address - 0xC000 ];
    }
    
    return ref;
}

uint8_t mem_t::cpu_memory_read( uint16_t address, bool peek )
{
    if ( address < 0x2000 )
    { // internal ram
        return cpu_mem.internal_ram[ address % 0x0800 ];
    }
    
    else if ( address < 0x4000 )
    { // ppu registers
        address = (address % 0x0008) + 0x2000;
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
                if (peek) return value;

                // Clear vblank status bit and mute it briefly
                ppu->vblank_suppression = true;
                ppu->regs.PPUSTATUS &= ~0x80;
                // Reset write toggle
                ppu_mem.w = 0;
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
                    addr %= 0x3F00;
                }
                ppu_mem.v.data = addr;
                return data;
            } break;
        }
    }

    else if ( address == 0x4014 ) 
    { // OAMDMA > write
        return ppu_mem.write_latch;
    }

    else if ( address == 0x4015 )
    { // APU Status 
        if (peek) return 0xFF; // NesTest workaround
        // Move to APU as well
        apu->status.r_pulse_1 = apu->pulse_1.length_counter > 0 && !apu->pulse_1.muted ? 1 : 0;
        apu->status.r_pulse_2 = apu->pulse_2.length_counter > 0 && !apu->pulse_2.muted ? 1 : 0;
        apu->status.r_triangle = apu->triangle.length_counter > 0 && !apu->triangle.muted ? 1 : 0;
        apu->status.r_noise = apu->noise.length_counter > 0 && !apu->noise.muted ? 1 : 0;
        apu->status.r_dmc = apu->dmc.memory_reader.bytes_remaining_counter > 0 ? 1 : 0;

        apu->status.r_frame_interrupt = apu->frame_interrupt;
        apu->status.r_dmc_interrupt = apu->dmc.interrupt_flag;

        LOG_W("Playing: %u (bytes left: %u) cycle: %u", apu->status.r_dmc, apu->dmc.memory_reader.bytes_remaining_counter, apu->cycle);
        
        uint8_t status = apu->status.data;
        apu->frame_interrupt = 0;
        return status;
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
        return mapper->cpu_read( address );
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

    else if ( address < 0x4000 )
    { // ppu registers
        address = (address % 0x0008) + 0x2000;
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
                ppu->frame_skip_suppression = true;
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
                    addr %= 0x3F00;
                }
                ppu_mem.v.data = addr;
                ppu_mem.write_latch = value;
            } break;
        }
    }

    else if ( address == 0x4014 )
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

        // The CPU is suspended during the transfer, which will take 513 or 514 cycles after the $4014 write tick.
        // (1 wait state cycle while waiting for writes to complete, +1 if on an odd CPU cycle, then 256 alternating read/write cycles.)
        memcpy( ppu_mem.oam.data, source, 256 );
        uint16_t wait_cycles = 513 + (cpu->cycles % 2) == 0 ? 0 : 1;
        cpu->tick_clock( wait_cycles );
        return;
    }

    else if ( address < 0x4020 )
    { // apu and I/O registers

        if ( address < 0x4004 )
        { // Pulse 1
            return apu->pulse_1.write( address, value );
        } else if ( address < 0x4008 )
        { // Pulse 2
            return apu->pulse_2.write( address, value );
        } else if ( address < 0x400C )
        { // Triangle
            return apu->triangle.write( address, value );
        } else if ( address < 0x4010 )
        { // Noise
            return apu->noise.write( address, value );
        } else if ( address < 0x4014 )
        { // DMC
            return apu->dmc.write( address, value );
        } else if ( address == 0x4015 )
        { // Status
            apu->status.data = value;
            
            // Clear DMC interrupt
            apu->dmc.interrupt_flag = false;

            if (apu->status.w_dmc)
            {
                apu->dmc.play = true;
            } else
            {
                apu->dmc.play = false;
                apu->dmc.memory_reader.bytes_remaining_counter = 0;
            }

            // TODO: Change in APU instead

            // Pulse 1
            if (apu->status.w_pulse_1 == 0) 
            {
                apu->pulse_1.muted = true;
                apu->pulse_1.length_counter = 0;
            } else {
                apu->pulse_1.muted = false;
            }

            // Pulse 2
            if (apu->status.w_pulse_2 == 0) 
            {
                apu->pulse_2.muted = true;
                apu->pulse_2.length_counter = 0;
            } else {
                apu->pulse_2.muted = false;
            }

            // Triangle
            if (apu->status.w_triangle == 0)
            {
                apu->triangle.muted = true;
                apu->triangle.length_counter = 0;
            } else {
                apu->triangle.muted = false;
            }

            // Noise
            if (apu->status.w_noise == 0) 
            {
                apu->noise.muted = true;
                apu->noise.length_counter = 0;
            } else {
                apu->noise.muted = false;
            }

        } else if ( address == 0x4017 )
        { // Frame Counter
            // TODO: Change in APU instead
            apu->frame_counter.data = value;
            apu->reset_frame_counter = apu->cycle % 2 ? 3 : 4;
            if (apu->frame_counter.interrupt_inhibit == 1) {
                apu->frame_interrupt = false;
            }
        }

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

        // Mute APU write warnings
        // LOG_W("Failed to write %02X @ $%04X (unmapped APU address space)", value, address);
        return;
    }

    else if ( address >= 0x4020 && address <= 0xFFFF )
    {
        mapper->cpu_write( address, value );
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
        return mapper->ppu_read( address );
    }

    else if ( address < 0x3F00 )
    { // nametables
        uint16_t t_addr = address % 0x2000;
        switch( ppu_mem.nt_mirroring )
        {
            case( ppu_mem_t::nametable_mirroring::horizontal ):
            {
                t_addr %= 0x0400;
                if ( address >= 0x2800 )
                {
                    t_addr += 0x0400;
                }
            } break;
            case( ppu_mem_t::nametable_mirroring::vertical ):
            {
                t_addr %= 0x800;
            } break;
            case( ppu_mem_t::nametable_mirroring::single_screen_lower ):
            {
                t_addr %= 0x400;
            } break;
            case( ppu_mem_t::nametable_mirroring::single_screen_higher ):
            {
                t_addr %= 0x400;
                t_addr += 0x400;
            } break;
            default:
            {
                LOG_E("Unimplemented nametable mirroring mode: %d", ppu_mem.nt_mirroring );
            } break;
        }
        return ppu_mem.vram[ t_addr ];
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

    else if ( address < 0x6000 )
    { // expansion rom?
        return cartridge_mem.expansion_rom[ address - 0x4020 ];
    }

    else if ( address < 0x8000 )
    { // sram?
        return cartridge_mem.sram[ address - 0x6000 ];
    }
    
    LOG_E("PPU memory read on weird address (%04X)", address );
    throw RESULT_ERROR;
}

void mem_t::ppu_memory_write( uint8_t value, uint16_t address )
{ // Writing to VRAM
    if ( address < 0x2000 )
    { // pattern tables
        mapper->ppu_write( address, value );
        return;
    }

    else if (address < 0x3F00) 
    { // nametables
        uint16_t t_addr = address % 0x2000;
        switch( ppu_mem.nt_mirroring )
        {
            case( ppu_mem_t::nametable_mirroring::horizontal ):
            {
                t_addr %= 0x0400;
                if ( address >= 0x2800 )
                {
                    t_addr += 0x0400;
                }
            } break;
            case( ppu_mem_t::nametable_mirroring::vertical ):
            {
                t_addr %= 0x800;
            } break;
            case( ppu_mem_t::nametable_mirroring::single_screen_lower ):
            {
                t_addr %= 0x400;
            } break;
            case( ppu_mem_t::nametable_mirroring::single_screen_higher ):
            {
                t_addr %= 0x400;
                t_addr += 0x400;
            } break;
            default:
            {
                LOG_E("Unimplemented nametable mirroring mode: %d", ppu_mem.nt_mirroring );
            } break;
        }
        ppu_mem.vram[ t_addr ] = value;
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