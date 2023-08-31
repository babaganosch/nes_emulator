#include <MiniFB.h>

#include "nes.hpp"
#include "logging.hpp"
#include "render.hpp"

namespace nes
{

namespace
{
static uint8_t color_2c02[] = { 
   0x80, 0x80, 0x80, 0x00, 0x3D, 0xA6, 0x00, 0x12, 0xB0, 0x44, 0x00, 0x96, 0xA1, 0x00, 0x5E,
   0xC7, 0x00, 0x28, 0xBA, 0x06, 0x00, 0x8C, 0x17, 0x00, 0x5C, 0x2F, 0x00, 0x10, 0x45, 0x00,
   0x05, 0x4A, 0x00, 0x00, 0x47, 0x2E, 0x00, 0x41, 0x66, 0x00, 0x00, 0x00, 0x05, 0x05, 0x05,
   0x05, 0x05, 0x05, 0xC7, 0xC7, 0xC7, 0x00, 0x77, 0xFF, 0x21, 0x55, 0xFF, 0x82, 0x37, 0xFA,
   0xEB, 0x2F, 0xB5, 0xFF, 0x29, 0x50, 0xFF, 0x22, 0x00, 0xD6, 0x32, 0x00, 0xC4, 0x62, 0x00,
   0x35, 0x80, 0x00, 0x05, 0x8F, 0x00, 0x00, 0x8A, 0x55, 0x00, 0x99, 0xCC, 0x21, 0x21, 0x21,
   0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0xFF, 0xFF, 0xFF, 0x0F, 0xD7, 0xFF, 0x69, 0xA2, 0xFF,
   0xD4, 0x80, 0xFF, 0xFF, 0x45, 0xF3, 0xFF, 0x61, 0x8B, 0xFF, 0x88, 0x33, 0xFF, 0x9C, 0x12,
   0xFA, 0xBC, 0x20, 0x9F, 0xE3, 0x0E, 0x2B, 0xF0, 0x35, 0x0C, 0xF0, 0xA4, 0x05, 0xFB, 0xFF,
   0x5E, 0x5E, 0x5E, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0xFF, 0xFF, 0xFF, 0xA6, 0xFC, 0xFF,
   0xB3, 0xEC, 0xFF, 0xDA, 0xAB, 0xEB, 0xFF, 0xA8, 0xF9, 0xFF, 0xAB, 0xB3, 0xFF, 0xD2, 0xB0,
   0xFF, 0xEF, 0xA6, 0xFF, 0xF7, 0x9C, 0xD7, 0xE8, 0x95, 0xA6, 0xED, 0xAF, 0xA2, 0xF2, 0xDA,
   0x99, 0xFF, 0xFC, 0xDD, 0xDD, 0xDD, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
};

inline uint8_t palette_id_to_red(uint32_t id) {
    return color_2c02[id*3];
}

inline uint8_t palette_id_to_green(uint32_t id) {
    return color_2c02[id*3+1];
}

inline uint8_t palette_id_to_blue(uint32_t id) {
    return color_2c02[id*3+2];
}
} // anonymous

RESULT ppu_t::init(mem_t &mem)
{
    memory = &mem;
    memory->ppu = this;
    recently_power_on = true;
    odd_frame = false;
    cycles = 0;
    x = 0;
    y = 261;
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
    if ( x > 340 )
    { // End of scanline, wrap around
        x = 0;
        y = (y + 1) % 262; // NTSC = 262 scanlines
    }
    
    ///////////////// Rendering

    if ( yy == 261 )
    { // Pre-render scanline (261)
        render_state = render_states::pre_render_scanline;
        if ( xx == 1 )
        { // vblank and sprite0 hit cleared at dot 1 of pre-render line.
            regs.PPUSTATUS &= ~0x80;
            regs.PPUSTATUS &= ~0x40;
        }
        if ( xx == 339 )
        { // Jump from (339, 261) to (0,0) on odd frames
            if ( odd_frame )
            {
                x = 0;
                y = 0;
            }
            odd_frame = !odd_frame;
        }
    }
    else if ( yy <= 239 )
    { // Visible scanlines (0-239)
        render_state = render_states::visible_scanline;
        if ( yy == 1 && xx == 1 )
        { // Fake sprite 0 at 1,1 for testing Mario Bros
            regs.PPUSTATUS |= 0x40;
        }
    }
    else if ( yy == 240 )
    { // Post-render scanline (240)
        render_state = render_states::post_render_scanline;
        // The PPU just idles during this scanline.
        // Even though accessing PPU memory from the program would be safe here, 
        // the VBlank flag isn't set until after this scanline.
    }
    else if ( yy <= 260 )
    { // Vertical blanking lines (241-260)
        render_state = render_states::vertical_blanking_line;
        if ( yy == 241 && xx == 1 )
        {
            // The VBlank flag of the PPU is set at tick 1 (the second tick) of
            // scanline 241, where the VBlank NMI also occurs.
            regs.PPUSTATUS |= 0x80;
            if (regs.PPUCTRL & 0x80) {
                memory->cpu->queue_nmi = true;
            }
        }
    }

    uint32_t bg_pixel = fetch_bg_pixel( xx, yy );
    uint32_t sp_pixel = fetch_sprite_pixel( xx, yy );
    (void) bg_pixel;
    window_buffer[ (yy * NES_WIDTH) + xx ] = bg_pixel;
    (void) sp_pixel;

    return RESULT_OK;
}

uint32_t ppu_t::fetch_bg_pixel( uint16_t dot, uint16_t scanline )
{
    if ( BIT_CHECK_LO(regs.PPUMASK, 3) )
    { // BG rendering disabled
        return 0x0;
    }
    
    uint32_t bg_color = 0x0;
    if ( render_state == render_states::visible_scanline && 
         dot < NES_WIDTH && scanline < NES_HEIGHT )
    { // Fetch current pixel color
        uint8_t fine_x = 7 - memory->ppu_mem.fine_x;
        uint8_t pattern_lo = shift_regs.pt_lo.lo;
        uint8_t pattern_hi = shift_regs.pt_hi.lo;
        uint8_t pattern = ((((pattern_lo >> fine_x) & 0x1) << 0) |
                           (((pattern_hi >> fine_x) & 0x1) << 1));

        uint8_t palette_lo = shift_regs.at_lo;
        uint8_t palette_hi = shift_regs.at_hi;
        uint8_t palette_id = ((((palette_lo >> fine_x) & 0x1) << 0) |
                              (((palette_hi >> fine_x) & 0x1) << 1));

        if (pattern == 0x0) 
        {
            uint32_t palette_bg = memory->ppu_mem.palette[0x00];
            bg_color = MFB_RGB(palette_id_to_red(palette_bg), palette_id_to_green(palette_bg), palette_id_to_blue(palette_bg));
        } 
        else 
        {
            uint8_t palette_set[3];
            palette_set[0] = memory->ppu_mem.palette[0x01+palette_id*4];
            palette_set[1] = memory->ppu_mem.palette[0x02+palette_id*4];
            palette_set[2] = memory->ppu_mem.palette[0x03+palette_id*4];
            bg_color = MFB_RGB(palette_id_to_red(palette_set[pattern-1]), palette_id_to_green(palette_set[pattern-1]), palette_id_to_blue(palette_set[pattern-1]));
        }
    }

    if (render_state == render_states::visible_scanline || 
        render_state == render_states::pre_render_scanline)
    {
        if ( dot == 0 )
        { // Idle cycle
            return bg_color;
        }

        if ( render_state == render_states::pre_render_scanline && 
           ( dot >= 280 && dot <= 304 ))
        {
            v_update_vert_v_eq_vert_t();
        }

        uint8_t eight_tick = dot % 8;
        if ( (dot > 0 && dot < 258) || (dot > 320 && dot < 337) )
        {
            if ( eight_tick == 1 )
            {
                reload_shift_registers();
            }
            shift_regs.pt_lo.data >>= 1;
            shift_regs.pt_hi.data >>= 1;
            // shift register for AT should probably be fed one bit
            // from the AT latch here. I'm feeding the whole byte 
            // instead each "reload" of shift registers (each tick 1)
        }
        switch (eight_tick)
        {
            case( 1 ):
            { // NT 1
                if ( dot == 257 )
                {
                    v_update_hori_v_eq_hori_t();
                }
                vram_fetch_nt( 0 );
            } break;
            case( 2 ):
            { // NT 2
                vram_fetch_nt( 1 );
            } break;
            case( 3 ):
            { // AT 1
                if ( (dot > 257 && dot < 320) || dot > 337 )
                { // Garbage NT
                    vram_fetch_nt( 0 );
                }
                else
                { // Regular AT
                    vram_fetch_at( 0 );
                }
            } break;
            case( 4 ):
            { // AT 2
                if ( (dot > 257 && dot < 320) || dot > 337 )
                { // Garbage NT
                    vram_fetch_nt( 1 );
                }
                else
                { // Regular AT
                    vram_fetch_at( 1 );
                }
            } break;
            case( 5 ):
            { // BG lsbits 1
                if ( dot < 257 || dot > 320 )
                {
                    vram_fetch_bg_lsbits( 0 );
                }
            } break;
            case( 6 ):
            { // BG lsbits 2
                if ( dot < 257 || dot > 320 )
                {
                    vram_fetch_bg_lsbits( 1 );
                }
            } break;
            case( 7 ):
            { // BG msbits 1
                if ( dot < 257 || dot > 320 )
                {
                    vram_fetch_bg_msbits( 0 );
                }
            } break;
            case( 0 ):
            { // BG msbits 2 & Update V
                if ( dot < 257 || dot > 320 )
                {
                    vram_fetch_bg_msbits( 1 );
                }
                if ( dot <= 256 || dot > 320 )
                {
                    v_update_inc_hori_v();
                }
                if ( dot == 256 )
                {
                    v_update_inc_vert_v();
                }
            } break;
        }
    }
    return bg_color;
}

void ppu_t::vram_fetch_nt( bool step )
{
    uint16_t &v = memory->ppu_mem.v.data;
    uint16_t t_addr = 0x2000 | (v & 0x0FFF);
    if ( step == 0 )
    {
        vram_address_multiplexer &= 0xFF00;
        vram_address_multiplexer |= t_addr & 0x00FF;
    }
    else
    {
        vram_address_multiplexer &= 0x00FF;
        vram_address_multiplexer |= t_addr & 0xFF00;
        latches.nt_byte = memory->ppu_memory_read( vram_address_multiplexer, false );
    }
}

void ppu_t::vram_fetch_at( bool step )
{
    uint16_t &v = memory->ppu_mem.v.data;
    uint16_t t_addr = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07);
    if ( step == 0 )
    {
        vram_address_multiplexer &= 0xFF00;
        vram_address_multiplexer |= t_addr & 0x00FF;
    }
    else
    {
        vram_address_multiplexer &= 0x00FF;
        vram_address_multiplexer |= t_addr & 0xFF00;
        latches.at_byte = memory->ppu_memory_read( vram_address_multiplexer, false );
    }
}

void ppu_t::vram_fetch_bg_lsbits( bool step )
{
    uint16_t t_addr = (latches.nt_byte * 16) + memory->ppu_mem.v.fine_y;
    if ( BIT_CHECK_HI(regs.PPUCTRL, 4) )
    {
        t_addr += 0x1000;
    }
    if ( step == 0 )
    {
        vram_address_multiplexer &= 0xFF00;
        vram_address_multiplexer |= t_addr & 0x00FF;
    }
    else
    {
        vram_address_multiplexer &= 0x00FF;
        vram_address_multiplexer |= t_addr & 0xFF00;
        latches.pt_tile = memory->ppu_memory_read( vram_address_multiplexer, false );
    }
}

void ppu_t::vram_fetch_bg_msbits( bool step )
{
    uint16_t t_addr = (latches.nt_byte * 16) + memory->ppu_mem.v.fine_y;
    if ( BIT_CHECK_HI(regs.PPUCTRL, 4) )
    {
        t_addr += 0x1000;
    }
    if ( step == 0 )
    {
        vram_address_multiplexer &= 0xFF00;
        vram_address_multiplexer |= t_addr & 0x00FF;
    }
    else
    {
        vram_address_multiplexer &= 0x00FF;
        vram_address_multiplexer |= t_addr & 0xFF00;
        latches.pt_tile |= ((uint16_t) memory->ppu_memory_read( vram_address_multiplexer + 8, false )) << 8;
    }
}

void ppu_t::v_update_inc_hori_v()
{
    if (++memory->ppu_mem.v.coarse_x == 0)
    {
        memory->ppu_mem.v.nametable ^= 0x01;
    }
}

void ppu_t::v_update_inc_vert_v()
{
    if (++memory->ppu_mem.v.fine_y == 0)
    {
        uint8_t temp_y = memory->ppu_mem.v.coarse_y;
        memory->ppu_mem.v.coarse_y++;
        if (temp_y == 29)
        {
            memory->ppu_mem.v.coarse_y = 0;
            memory->ppu_mem.v.nametable ^= 0x10;
        }
    }
}

void ppu_t::v_update_hori_v_eq_hori_t()
{
    // v: ....A.. ...BCDEF <- t: ....A.. ...BCDEF
    uint16_t mask = 0b0111101111100000; // (note: 15 bits, not 16)
    memory->ppu_mem.v.data &= mask;
    memory->ppu_mem.v.data |= (memory->ppu_mem.t.data & ~mask);
}

void ppu_t::v_update_vert_v_eq_vert_t()
{
    // v: GHIA.BC DEF..... <- t: GHIA.BC DEF.....
    uint16_t mask = 0b0000010000011111; // (note: 15 bits, not 16)
    memory->ppu_mem.v.data &= mask;
    memory->ppu_mem.v.data |= (memory->ppu_mem.t.data & ~mask);
}

void ppu_t::reload_shift_registers()
{
    // A and B?
    shift_regs.pt_hi.hi = (latches.pt_tile & 0xFF00) >> 8;
    shift_regs.pt_lo.hi =  latches.pt_tile & 0x00FF;
    // P
    shift_regs.at_hi = (latches.at_byte & 0b10) > 0 ? 0xFF : 0x00;
    shift_regs.at_lo = (latches.at_byte & 0b01) > 0 ? 0xFF : 0x00;
}

uint32_t ppu_t::fetch_sprite_pixel( uint16_t dot, uint16_t scanline )
{
    (void) dot;
    (void) scanline;
    return 0x00;
}

} // nes