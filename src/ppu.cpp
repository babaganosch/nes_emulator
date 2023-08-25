#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

RESULT ppu_t::init(mem_t &mem)
{
    memory = &mem;
    memory->ppu = this;
    recently_power_on = true;
    odd_frame = false;
    cycles = 0;
    x = 0;
    y = 0;
    regs.PPUCTRL = 0x00;
    regs.PPUMASK = 0x00;
    regs.PPUSTATUS = 0x00;
    regs.OAMADDR = 0x00;

    return RESULT_OK;
}

RESULT ppu_t::execute()
{
    cycles++;

    uint16_t xx = x++;   // Each scanline range from 0 to 340 dots
    uint16_t yy = y;     // Scanlines range from 0 to 261
    if (x > 340)
    { // End of scanline, wrap around
        x = 0;
        y = (y + 1) % 262; // NTSC = 262 scanlines
    }
    
    ///////////////// Rendering

    if ( yy == 261 )
    { // Pre-render scanline (261)
        if ( xx == 1 )
        { // vblank and sprite0 hit cleared at dot 1 of pre-render line.
            regs.PPUSTATUS &= ~0x80;
            regs.PPUSTATUS &= ~0x40;
        }
        return RESULT_OK;
    }
    else if ( yy <= 239 )
    { // Visible scanlines (0-239)
        
        if ( yy == 1 && xx == 1 )
        { // Fake sprite 0 at 1,1 for testing Mario Bros
            regs.PPUSTATUS |= 0x40;
        }
        
        return RESULT_OK;
    }
    else if ( yy == 240 )
    { // Post-render scanline (240)
        // The PPU just idles during this scanline.
        // Even though accessing PPU memory from the program would be safe here, 
        // the VBlank flag isn't set until after this scanline.
        return RESULT_OK;
    }
    else if ( yy <= 260 )
    { // Vertical blanking lines (241-260)
        if ( yy == 241 && xx == 1 )
        {
            // The VBlank flag of the PPU is set at tick 1 (the second tick) of
            // scanline 241, where the VBlank NMI also occurs.
            regs.PPUSTATUS |= 0x80;
            if (regs.PPUCTRL & 0x80) {
                memory->cpu->queue_nmi = true;
            }
        }
        return RESULT_OK;
    }

    return RESULT_ERROR;
}

} // nes