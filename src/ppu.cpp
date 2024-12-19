#include <MiniFB.h>

#include "nes.hpp"
#include "logging.hpp"

/*
* There's a lot of magical numbers in this file related to dot and scanline timings.
* These numbers are based on a timing diagram found on nesdev.org
* https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
*/

namespace nes
{

namespace
{
const uint8_t color_2c02[] = { 
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

const uint8_t reverse_byte_lookup_table[] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

inline uint8_t reverse_byte(uint8_t x)
{
    return reverse_byte_lookup_table[x];
}

inline uint8_t palette_id_to_red(uint32_t id)
{
    return color_2c02[id*3];
}

inline uint8_t palette_id_to_green(uint32_t id)
{
    return color_2c02[id*3+1];
}

inline uint8_t palette_id_to_blue(uint32_t id)
{
    return color_2c02[id*3+2];
}
} // anonymous

uint32_t window_buffer[NES_WIDTH * NES_HEIGHT * 4];

RESULT ppu_t::init(mem_t &mem)
{
    memory = &mem;
    memory->ppu = this;
    recently_power_on = true;
    frame_num = 0;
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

    uint16_t dot = x++;    // Each scanline range from 0 to 340 dots
    uint16_t scanline = y; // Scanlines range from 0 to 261
    if ( x > 340 )
    { // End of scanline, wrap around
        x = 0;
        y = (y + 1) % 262; // NTSC = 262 scanlines
    }
    
    if ( scanline == 261 )
    { // Pre-render scanline (261)
        render_state = render_states::pre_render_scanline;
        if ( dot == 1 )
        { // vblank and sprite0 hit cleared at dot 1 of pre-render line.
            regs.PPUSTATUS &= ~0x80;
            regs.PPUSTATUS &= ~0x40;
        }
        if ( dot == 339 )
        { // Jump from (339, 261) to (0,0) on odd frames
            if ( BIT_CHECK_HI(regs.PPUMASK, 3) && ((frame_num++ % 2) != 0) )
            {
                x = 0;
                y = 0;
            }
        }
    }
    else if ( scanline <= 239 )
    { // Visible scanlines (0-239)
        render_state = render_states::visible_scanline;
    }
    else if ( scanline == 240 )
    { // Post-render scanline (240)
        render_state = render_states::post_render_scanline;
        // The PPU just idles during this scanline.
        // Even though accessing PPU memory from the program would be safe here, 
        // the VBlank flag isn't set until after this scanline.
    }
    else if ( scanline <= 260 )
    { // Vertical blanking lines (241-260)
        render_state = render_states::vertical_blanking_line;
        if ( scanline == 241 && dot == 1 )
        {
            // The VBlank flag of the PPU is set at tick 1 (the second tick) of
            // scanline 241, where the VBlank NMI also occurs.
            regs.PPUSTATUS |= 0x80;
            if (regs.PPUCTRL & 0x80) {
                memory->cpu->nmi_pending = true;
            }
        }
    }

    bg_evaluation( dot, scanline );
    sp_evaluation( dot, scanline );
    render_pixel( dot, scanline );
    
    return RESULT_OK;
}

void ppu_t::bg_evaluation( uint16_t dot, uint16_t scanline )
{
    if ( (BIT_CHECK_LO(regs.PPUMASK, 3)) || render_state == render_states::post_render_scanline )
    { // BG rendering disabled
        return;
    }

    if ((render_state == render_states::visible_scanline || 
         render_state == render_states::pre_render_scanline) && dot != 0)
    {
        if ( render_state == render_states::pre_render_scanline )
        {
            if ( dot >= 280 && dot <= 304 )
            {
                v_update_vert_v_eq_vert_t();
            }
        } 

        uint8_t eight_tick = dot % 8;
        switch ( eight_tick )
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
            { // BG msbits 2 & Update V & Reload Shift Registers
                if ( dot < 257 || dot > 320 )
                {
                    vram_fetch_bg_msbits( 1 );
                    v_update_inc_hori_v();
                    if ( dot == 256 )
                    {
                        v_update_inc_vert_v();
                    }
                }
                if ( dot < 257 || (dot > 320 && dot < 337) )
                {
                    reload_shift_registers();
                }
            } break;
        }
    }
}

void ppu_t::vram_fetch_nt( bool step )
{
    uint16_t &v = memory->ppu_mem.v.data;
    uint16_t t_addr = 0x2000 | (v & 0x0FFF);
    if ( step == 0 )
    {
        vram_address_multiplexer = (vram_address_multiplexer & 0xFF00) | (t_addr & 0x00FF);
    }
    else
    {
        vram_address_multiplexer = (vram_address_multiplexer & 0x00FF) | (t_addr & 0xFF00);
        latches.nt_latch = memory->ppu_memory_read( vram_address_multiplexer, false );
    }
}

void ppu_t::vram_fetch_at( bool step )
{
    uint16_t &v = memory->ppu_mem.v.data;
    uint16_t t_addr = 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x0038) | ((v >> 2) & 0x0007);
    if ( step == 0 )
    {
        vram_address_multiplexer = (vram_address_multiplexer & 0xFF00) | (t_addr & 0x00FF);
    }
    else
    {
        vram_address_multiplexer = (vram_address_multiplexer & 0x00FF) | (t_addr & 0xFF00);
        latches.at_byte = memory->ppu_memory_read( vram_address_multiplexer, false );
        if (memory->ppu_mem.v.coarse_y & 2) latches.at_byte >>= 4;
        if (memory->ppu_mem.v.coarse_x & 2) latches.at_byte >>= 2;
    }
}

void ppu_t::vram_fetch_bg_lsbits( bool step )
{
    uint16_t t_addr = (latches.nt_latch * 16) + memory->ppu_mem.v.fine_y;
    if ( BIT_CHECK_HI(regs.PPUCTRL, 4) )
    {
        t_addr += 0x1000;
    }
    if ( step == 0 )
    {
        vram_address_multiplexer = (vram_address_multiplexer & 0xFF00) | (t_addr & 0x00FF);
    }
    else
    {
        vram_address_multiplexer = (vram_address_multiplexer & 0x00FF) | (t_addr & 0xFF00);
        latches.pt_latch = memory->ppu_memory_read( vram_address_multiplexer, false );
    }
}

void ppu_t::vram_fetch_bg_msbits( bool step )
{
    uint16_t t_addr = (latches.nt_latch * 16) + memory->ppu_mem.v.fine_y + 8;
    if ( BIT_CHECK_HI(regs.PPUCTRL, 4) )
    {
        t_addr += 0x1000;
    }
    if ( step == 0 )
    {
        vram_address_multiplexer = (vram_address_multiplexer & 0xFF00) | (t_addr & 0x00FF);
    }
    else
    {
        vram_address_multiplexer = (vram_address_multiplexer & 0x00FF) | (t_addr & 0xFF00);
        latches.pt_latch |= ((uint16_t) memory->ppu_memory_read( vram_address_multiplexer, false )) << 8;
    }
}

void ppu_t::v_update_inc_hori_v()
{
    if (++memory->ppu_mem.v.coarse_x == 0)
    {
        memory->ppu_mem.v.nametable ^= 0b01;
    }
}

void ppu_t::v_update_inc_vert_v()
{
    if (++memory->ppu_mem.v.fine_y == 0)
    {
        if (memory->ppu_mem.v.coarse_y++ == 29)
        {
            memory->ppu_mem.v.coarse_y = 0;
            memory->ppu_mem.v.nametable ^= 0b10;
        }
    }
}

void ppu_t::v_update_hori_v_eq_hori_t()
{
    // v: ....A.. ...BCDEF <- t: ....A.. ...BCDEF
    uint16_t mask = 0b1111101111100000; // (note: 15 bits, not 16.  Highest bit is 1 here)
    memory->ppu_mem.v.data &= mask;
    memory->ppu_mem.v.data |= (memory->ppu_mem.t.data & ~mask);
}

void ppu_t::v_update_vert_v_eq_vert_t()
{
    // v: GHIA.BC DEF..... <- t: GHIA.BC DEF.....
    uint16_t mask = 0b1000010000011111; // (note: 15 bits, not 16.  Highest bit is 1 here)
    memory->ppu_mem.v.data &= mask;
    memory->ppu_mem.v.data |= (memory->ppu_mem.t.data & ~mask);
}

void ppu_t::reload_shift_registers()
{
    // A and B?
    shift_regs.pt_hi.lo = (latches.pt_latch & 0xFF00) >> 8;
    shift_regs.pt_lo.lo =  latches.pt_latch & 0x00FF;
    // P
    latches.at_latch = latches.at_byte & 0b11;
}

void ppu_t::sp_evaluation( uint16_t dot, uint16_t scanline )
{
    if ( BIT_CHECK_LO(regs.PPUMASK, 4) || dot == 0 ||
        (render_state != render_states::visible_scanline && render_state != render_states::pre_render_scanline) )
    { // BG rendering disabled or idle cycle
        return;
    }

    // Sprite byte structure
    // [0] y-coordinate
    // [1] tile-index
    // [2] attribute data
    // [3] x-coordinate
    oam_t& oam = memory->ppu_mem.oam;
    soam_t& soam = memory->ppu_mem.soam;

    if ( render_state == render_states::visible_scanline )
    { 
        // Secondary OAM clear and sprite evaluation for next scanline

        if ( dot == 65 )
        { // Reset stuff
            oam_n = 0; // n: Sprite [ 0 - 63 ]
            oam_m = 0; // m: Byte   [ 0 -  3 ]
            soam_counter = 0; // sOAM index [ 0 - 7 ]
            sprite_fetch = 0; // counter used for sprite fetching
            memset( sprite_indices_next_scanline, 0xFF, sizeof(uint8_t) * 8 );
        }

        if ( dot <= 64 )
        { // Secondary OAM clear
            soam.data[ (dot - 1) / 2 ] = 0xFF;
        }

        else if ( dot <= NES_WIDTH )
        { // Sprite evaluation (dot 65-256)

            if ( (dot % 2) == 0 )
            { // Even cycle
                // data is written to secondary OAM (unless secondary OAM is full, 
                // in which case it will read the value in secondary OAM instead)
                if ( oam_n < 64 )
                {
                    // 1. Starting at n = 0, read a sprite's Y-coordinate (OAM[n][0], 
                    //    copying it to the next open slot in secondary OAM (unless 8 sprites
                    //    have been found, in which case the write is ignored).
                    if ( soam_counter < 8 )
                    {
                        soam.arr2d[ soam_counter ][0] = oam_read_buffer[0];

                        // 1a. If Y-coordinate is in range, copy remaining bytes of 
                        //     sprite data (OAM[n][1] thru OAM[n][3]) into secondary OAM.
                        uint8_t& yy = soam.arr2d[ soam_counter ][0];
                        
                        bool is_8x16 = BIT_CHECK_HI(regs.PPUCTRL, 5);

                        if ( (scanline >= yy) && (scanline <= (yy + (is_8x16 ? 15 : 7))) )
                        {
                            soam.arr2d[ soam_counter ][1] = oam_read_buffer[1];
                            soam.arr2d[ soam_counter ][2] = oam_read_buffer[2];
                            soam.arr2d[ soam_counter ][3] = oam_read_buffer[3];

                            if (is_8x16) {
                                uint8_t& at = soam.arr2d[ soam_counter ][2];
                                bool flip_y = BIT_CHECK_HI(at, 7);

                                if (scanline > yy + 7) 
                                { // bot tile
                                    yy -= 8;
                                    if (flip_y) {
                                        yy += 16;
                                    }
                                } 
                                else if (scanline >= yy) 
                                { // top tile
                                    if (flip_y) {
                                        yy += 16;
                                    }
                                }
                            }

                            // Store sprite index for sprite 0 checking
                            sprite_indices_next_scanline[ soam_counter ] = oam_n;
                            soam_counter++;
                        }
                    }

                    // 2. Increment n
                    oam_n++;

                    // 2a. If n has overflowed back to zero (all 64 sprites evaluated), go to 4
                    if ( oam_n >= 64 )
                    { }

                    // 2b. If less than 8 sprites have been found, go to 1
                    if ( soam_counter < 8 )
                    { }

                    // 2c. If exactly 8 sprites have been found, disable writes to secondary
                    //     OAM because it is full. This causes sprites in back to drop out.

                    // 3. Starting at m = 0, evaluate OAM[n][m] as a Y-coordinate.
                    // 3a. If the value is in range, set the sprite overflow flag in $2002 and read the
                    //     next 3 entries of OAM (incrementing 'm' after each byte and incrementing 'n'
                    //     when 'm' overflows); if m = 3, increment n
                    // 3b. If the value is not in range, increment n and m (without carry). If n overflows
                    //     to 0, go to 4; otherwise go to 3
                    //     The m increment is a hardware bug - if only n was incremented, the overflow flag
                    //     would be set whenever more than 8 sprites were present on the same scanline, as expected.
                
                    // 4. Attempt (and fail) to copy OAM[n][0] into the next free slot in secondary OAM, and
                    //    increment n (repeat until HBLANK is reached)
                }

            }
            else
            { // Uneven cycle
                // Data is read from pOAM
                memcpy( oam_read_buffer, oam.arr2d[ oam_n ], sizeof(u_int8_t) * 4 );
            }   

        }

        if ( dot == 257 )
        { // Clear sprite indices for old scanline
            memset( sprite_indices_current_scanline, 0xFF, sizeof(uint8_t) * 8 );
        }
    }

    if ( render_state == render_states::visible_scanline || render_state == render_states::pre_render_scanline )
    {
        if ( dot > 256 && dot < 321 )
        { // Cycles 257-320: Sprite fetches (8 sprites total, 8 cycles per sprite)
            uint8_t eight_tick = dot % 8;
            switch ( eight_tick )
            {
                case( 5 ):
                { // Sprite lsbits 1
                    uint8_t sprite_y    = soam.arr2d[ sprite_fetch ][0]; // y
                    uint8_t sprite_tile = soam.arr2d[ sprite_fetch ][1]; // tile
                    uint8_t sprite_attr = soam.arr2d[ sprite_fetch ][2]; // attr
                    uint8_t sprite_x    = soam.arr2d[ sprite_fetch ][3]; // x

                    // Store current scanlines sprite indices for sprite 0 hit checks
                    sprite_indices_current_scanline[ sprite_fetch ] = sprite_indices_next_scanline[ sprite_fetch ];

                    // Set X counter for sprite
                    sprite_counters[ sprite_fetch ] = sprite_x + 1;

                    // Fill attribute latch for sprite
                    latches.sprite_attribute_latch[ sprite_fetch ] = sprite_attr;

                    // Calculate pattern tables for sprite
                    bool flip_x = BIT_CHECK_HI(sprite_attr, 6);
                    bool flip_y = BIT_CHECK_HI(sprite_attr, 7);

                    uint16_t chr_offset = 0x0;
                    bool is_8x16 = BIT_CHECK_HI(regs.PPUCTRL, 5);
                    if (is_8x16) 
                    {
                        if (BIT_CHECK_HI(sprite_tile, 0))
                        {
                            chr_offset = 0x1000;
                        }
                        sprite_tile &= 0xFE;
                    } 
                    else
                    {
                        if (BIT_CHECK_HI(regs.PPUCTRL, 3))
                        {
                            chr_offset = 0x1000;
                        }
                    }

                    uint8_t y_offset = (scanline - sprite_y);
                    if (flip_y) 
                    {
                        y_offset = 7 - y_offset;
                    }

                    uint8_t* chr_data = (uint8_t*)memory->cartridge_mem.chr_rom.data + (sprite_tile*16) + chr_offset + y_offset;
                    uint8_t lo = *chr_data;
                    uint8_t hi = *(chr_data+8);

                    // Fill shift-registers..
                    if (flip_x)
                    {
                        shift_regs.sprite_pattern_tables_lo[ sprite_fetch ] = reverse_byte( lo );
                        shift_regs.sprite_pattern_tables_hi[ sprite_fetch ] = reverse_byte( hi );
                    }
                    else
                    {
                        shift_regs.sprite_pattern_tables_lo[ sprite_fetch ] = lo;
                        shift_regs.sprite_pattern_tables_hi[ sprite_fetch ] = hi;
                    }

                } break;
                case( 6 ):
                { // Sprite lsbits 2

                } break;
                case( 7 ):
                { // Sprite msbits 1
                    ++sprite_fetch %= 8;
                } break;
                case( 0 ):
                { // Sprite msbits 2

                } break;
            }
        }
    }
}

void ppu_t::render_pixel( uint16_t dot, uint16_t scanline )
{
    uint32_t bg_color{0};
    uint32_t bg_pattern{0};
    uint32_t sp_color{0};

    // Background pixel color
    if ( dot < NES_WIDTH && scanline < NES_HEIGHT )
    {
        
        uint8_t fine_x = 7 - memory->ppu_mem.fine_x;
        bg_pattern = ((((shift_regs.pt_lo.hi >> fine_x) & 0x1) << 0) |
                      (((shift_regs.pt_hi.hi >> fine_x) & 0x1) << 1));

        uint8_t palette_lo = shift_regs.at_lo;
        uint8_t palette_hi = shift_regs.at_hi;
        uint8_t palette_id = ((((palette_lo >> fine_x) & 0x1) << 0) |
                              (((palette_hi >> fine_x) & 0x1) << 1));

        if ( bg_pattern == 0x0 ) 
        {
            uint32_t palette_bg = memory->ppu_mem.palette[0x00];
            bg_color = MFB_RGB(palette_id_to_red(palette_bg),
                            palette_id_to_green(palette_bg),
                            palette_id_to_blue(palette_bg));
        } 
        else 
        {
            uint8_t palette_set[3];
            palette_set[0] = memory->ppu_mem.palette[0x01+palette_id*4];
            palette_set[1] = memory->ppu_mem.palette[0x02+palette_id*4];
            palette_set[2] = memory->ppu_mem.palette[0x03+palette_id*4];
            bg_color = MFB_RGB(palette_id_to_red(palette_set[bg_pattern-1]),
                            palette_id_to_green(palette_set[bg_pattern-1]),
                            palette_id_to_blue(palette_set[bg_pattern-1]));
        }
    }

    // Shifting of shift registers
    if ( (dot < NES_WIDTH && scanline < NES_HEIGHT) || (dot > 320 && dot < 336) )
    {
        // Shift BG registers
        shift_regs.pt_lo.data <<= 1;
        shift_regs.pt_hi.data <<= 1;
        shift_regs.at_hi <<= 1;
        shift_regs.at_lo <<= 1;
        // Pull in bit from AT bit-latches
        shift_regs.at_hi |= ((latches.at_latch & 0b10) >> 1);
        shift_regs.at_lo |= (latches.at_latch & 0b01);
    }

    if ( BIT_CHECK_LO(regs.PPUMASK, 1) && dot < 8 )
    { // Leftmost 8-pixel mask for BGs
        bg_color = 0x0;
        bg_pattern = 0x0;
    }

    // Sprites pixel color
    if ( dot <= NES_WIDTH )
    {
        for (auto sprite = 0; sprite < 8; ++sprite)
        {
            int16_t& sprite_x_counter = sprite_counters[ sprite ];
            if ( sprite_x_counter != 0xFF)
            { // Dec the counter
                sprite_x_counter--;
            }
        }
    }
    
    sp_color = 0x0;
    bool sprite_hit = false;
    uint8_t sp_to_bg_priority = 0x1;
    if ( render_state == render_states::visible_scanline )
    {
        for (auto sprite = 0; sprite < 8; ++sprite)
        {
            int16_t sprite_x_counter = sprite_counters[ sprite ];
            
            if ( sprite_x_counter <= 0 )
            { // Trigger!
                bool lo_bit = BIT_CHECK_HI(shift_regs.sprite_pattern_tables_lo[ sprite ], 7);
                bool hi_bit = BIT_CHECK_HI(shift_regs.sprite_pattern_tables_hi[ sprite ], 7);
                shift_regs.sprite_pattern_tables_lo[ sprite ] <<= 1;
                shift_regs.sprite_pattern_tables_hi[ sprite ] <<= 1;

                uint8_t pattern = lo_bit | (hi_bit << 1);

                if ( BIT_CHECK_LO(regs.PPUSTATUS, 6) && pattern && sprite_indices_current_scanline[ sprite ] == 0 )
                { // Sprite Zero check
                    regs.PPUSTATUS |= 0x40;
                }
                
                if ( !sprite_hit && pattern )
                {
                    sprite_hit = true;
                    uint8_t palette_id = latches.sprite_attribute_latch[ sprite ] & 0b11;
                    sp_to_bg_priority = BIT_CHECK_HI(latches.sprite_attribute_latch[ sprite ], 5);

                    static uint8_t palette_set[3];
                    palette_set[0] = memory->ppu_mem.palette[0x11+palette_id*4];
                    palette_set[1] = memory->ppu_mem.palette[0x12+palette_id*4];
                    palette_set[2] = memory->ppu_mem.palette[0x13+palette_id*4];

                    sp_color = MFB_RGB(palette_id_to_red(palette_set[pattern-1]), 
                                       palette_id_to_green(palette_set[pattern-1]),
                                       palette_id_to_blue(palette_set[pattern-1]));

                }
            }
        }
    }

    if ( BIT_CHECK_LO(regs.PPUMASK, 2) && dot < 8 )
    { // Leftmost 8-pixel mask for sprites
        sp_color = 0x0;
    }

    // Priority multiplexing
    uint32_t color = 0x0;
    if ( bg_pattern == 0x0 && sp_color == 0x0 )
    { // BG ($3F00)
        color = bg_color;
    }
    else if ( bg_pattern == 0x0 && sp_color != 0x0 )
    { // Sprite
        color = sp_color;
    }
    else if ( bg_pattern != 0x0 && sp_color == 0x0 )
    { // BG
        color = bg_color;
    }
    else
    { // Priority check
        if ( sp_to_bg_priority == 0 )
        { // Foreground
            color = sp_color;
        }
        else
        { // Background
            color = bg_color;
        }
    }

    // Render Pixel
    if ( dot < NES_WIDTH && scanline < NES_HEIGHT )
    {
        uint32_t pixel_index = (scanline * NES_WIDTH) + dot;
        pixel_index = pixel_index % (NES_WIDTH * NES_HEIGHT);
        window_buffer[ pixel_index ] = color;
    }
    
}

} // nes