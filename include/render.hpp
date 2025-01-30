#ifndef RENDER_HPP
#define RENDER_HPP

#include "nes.hpp"

namespace nes
{

extern uint32_t nt_window_buffer[];

void dump_nametables(emu_t &emu);
void dump_sprites(emu_t &emu);
void dump_ppu_vram(emu_t& emu);

void clear_framebuffer(uint32_t* framebuffer, uint8_t r, uint8_t g, uint8_t b);
void draw_glyph(uint32_t* framebuffer, uint32_t x, uint32_t y, uint8_t glyph);
void draw_text(uint32_t* framebuffer, uint32_t x, uint32_t y, const char* text, ...);

} // nes

#endif /* RENDER_HPP */