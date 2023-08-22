#ifndef RENDER_HPP
#define RENDER_HPP

#include "nes.hpp"
#include "render.hpp"

namespace nes
{

extern uint32_t window_buffer[];

void dump_nametables(emu_t &emu);
void dump_sprites(emu_t &emu);

void clear_window_buffer(uint8_t r, uint8_t g, uint8_t b);
void draw_glyph(uint32_t x, uint32_t y, uint8_t glyph);
void draw_text(uint32_t x, uint32_t y, const char* text, ...);

} // nes

#endif /* RENDER_HPP */