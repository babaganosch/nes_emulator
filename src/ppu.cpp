#include "nes.hpp"

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
    write_latch = 0x00;
    regs.PPUCTRL = 0x00;
    regs.PPUMASK = 0x00;
    regs.PPUSTATUS = 0x00;
    regs.OAMADDR = 0x00;
    regs.PPUSCROLL[0] = 0x00;
    regs.PPUSCROLL[1] = 0x00;
    regs.PPUADDR[0] = 0x00;
    regs.PPUADDR[1] = 0x00;
    regs.PPUDATA = 0x00;

    return RESULT_OK;
}

RESULT ppu_t::execute()
{
    cycles++;

    int16_t xx = x++;   // Each scanline range from 0 to 340 dots
    int16_t yy = y - 1; // Scanlines range from -1 to 261

    // Check if in pre-render line
    if ((yy == 261 || yy == -1) && xx == 1) {
        regs.PPUSTATUS &= ~0x80;
        // Sprite 0 Hit flag is cleared at dot 1 of the pre-render line.
        regs.PPUSTATUS &= ~0x40;
    }

    if (x > 340)
    { // End of scanline, wrap around
        x = 0;
        y = (y + 1) % 262; // NTSC = 262 scanlines
    }

    if ( yy == 1 && xx == 1)
    { // Fake sprite 0 at 1,1 for testing Mario Bros
        regs.PPUSTATUS |= 0x40;
    }

    if ( yy == 240 )
    { // Post-render scanline
        // The PPU just idles during this scanline.
        // Even though accessing PPU memory from the program would be safe here, 
        // the VBlank flag isn't set until after this scanline.
    }
  
    if ( yy == 241 && xx == 1 ) 
    {
        // Vertical blank has started (0: not in vblank; 1: in vblank).
        // Set at dot 1 of line 241 (the line *after* the post-render
        // line); cleared after reading $2002 and at dot 1 of the
        // pre-render line.
        regs.PPUSTATUS |= 0x80;

        // PPUCTRL Bit 8; Generate an NMI at the start of the vertical blanking interval (0: off; 1: on)
        if (regs.PPUCTRL & 0x80) {
            memory->cpu->queue_nmi = true;
        }
    }

    return RESULT_OK;
}

} // nes