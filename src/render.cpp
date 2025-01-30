#include <MiniFB.h>

#include "nes.hpp"
#include "render.hpp"
#include "logging.hpp"


/*
 * NOTE: A lot of code here is taken from nespresso by Sven Andersson,
 *       to get things showing on screen.
 *       https://github.com/andsve/nespresso
 */

namespace nes
{

uint32_t nt_window_buffer[(NES_WIDTH*2) * (NES_HEIGHT*2) * 4];

static uint8_t font8x8_basic[128][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0000 (nul)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0001
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0002
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0003
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0004
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0005
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0006
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0007
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0008
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0009
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0010
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0011
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0012
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0013
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0014
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0015
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0016
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0017
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0018
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0019
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0020 (space)
    { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // U+0021 (!)
    { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0022 (")
    { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   // U+0023 (#)
    { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   // U+0024 ($)
    { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   // U+0025 (%)
    { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   // U+0026 (&)
    { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0027 (')
    { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   // U+0028 (()
    { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   // U+0029 ())
    { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // U+002A (*)
    { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   // U+002B (+)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+002C (,)
    { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   // U+002D (-)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+002E (.)
    { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   // U+002F (/)
    { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
    { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
    { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
    { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
    { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
    { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
    { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
    { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
    { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
    { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+003A (:)
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+003B (;)
    { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   // U+003C (<)
    { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   // U+003D (=)
    { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+003E (>)
    { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   // U+003F (?)
    { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   // U+0040 (@)
    { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0041 (A)
    { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0042 (B)
    { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // U+0043 (C)
    { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // U+0044 (D)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0045 (E)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // U+0046 (F)
    { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // U+0047 (G)
    { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0048 (H)
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0049 (I)
    { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // U+004A (J)
    { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+004B (K)
    { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // U+004C (L)
    { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+004D (M)
    { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+004E (N)
    { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+004F (O)
    { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+0050 (P)
    { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // U+0051 (Q)
    { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // U+0052 (R)
    { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // U+0053 (S)
    { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0054 (T)
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U+0055 (U)
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0056 (V)
    { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // U+0057 (W)
    { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+0058 (X)
    { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+0059 (Y)
    { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+005A (Z)
    { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   // U+005B ([)
    { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   // U+005C (\)
    { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   // U+005D (])
    { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   // U+005E (^)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+005F (_)
    { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0060 (`)
    { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   // U+0061 (a)
    { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   // U+0062 (b)
    { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   // U+0063 (c)
    { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   // U+0064 (d)
    { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   // U+0065 (e)
    { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   // U+0066 (f)
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0067 (g)
    { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   // U+0068 (h)
    { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0069 (i)
    { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   // U+006A (j)
    { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   // U+006B (k)
    { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+006C (l)
    { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   // U+006D (m)
    { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   // U+006E (n)
    { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+006F (o)
    { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+0070 (p)
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   // U+0071 (q)
    { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   // U+0072 (r)
    { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   // U+0073 (s)
    { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0074 (t)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   // U+0075 (u)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0076 (v)
    { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   // U+0077 (w)
    { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   // U+0078 (x)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0079 (y)
    { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   // U+007A (z)
    { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   // U+007B ({)
    { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+007C (|)
    { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   // U+007D (})
    { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+007E (~)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+007F
};

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

static uint8_t gray_palette[] = { 
    0,   0,   0,
    85,  85,  85,
    170, 170, 170,
    255, 255, 255
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

static void ppu_get_chr(emu_t& emu, uint16_t address, uint8_t* out_data, bool bg)
{
    uint16_t chr_offset = 0x0;
    if (bg) {
        if ((emu.ppu.regs.PPUCTRL >> 4) & 0x1) {
            chr_offset = 0x1000;
        }
    } else {
        if ((emu.ppu.regs.PPUCTRL >> 3) & 0x1) {
            chr_offset = 0x1000;
        }
    }

    uint8_t* chr_data = (uint8_t*)emu.memory.cartridge_mem.chr_rom.data + address + chr_offset;

    // low bits
    for (uint32_t y = 0; y < 8; ++y)
    {
        uint8_t d = *chr_data;
        for (uint32_t x = 0; x < 8; ++x)
        {
            // clear pix
            uint32_t pix_i = (y*8) + (7-x);
            out_data[pix_i] = 0x0;

            out_data[pix_i] = (d >> x) & 0x1;
        }
        chr_data++;
    }

    // high bits
    for (uint32_t y = 0; y < 8; ++y)
    {
        uint8_t d = *chr_data;
        for (uint32_t x = 0; x < 8; ++x)
        {
            // clear pix
            uint32_t pix_i = (y*8) + (7-x);

            out_data[pix_i] = out_data[pix_i] | (((d >> x) & 0x1) << 1);
        }
        chr_data++;
    }
}

static void blit_chr(emu_t& emu, uint32_t pix_x, uint32_t pix_y, uint32_t chr_index, bool bg, bool flip_x, bool flip_y, uint8_t* palette_set)
{
    uint8_t chr[8*8]; // 8x8 pixels is one character (tile)
    ppu_get_chr(emu, chr_index*16, chr, bg);

    for (uint32_t y = 0; y < 8; ++y)
    {
        for (uint32_t x = 0; x < 8; ++x)
        {
            uint8_t pix = chr[y*8+x];

            if (!bg && pix == 0x0)
                continue;
                
            uint32_t tx = pix_x + (flip_x ? 7 - x : x);
            uint32_t ty = pix_y + (flip_y ? 7 - y : y);

            uint32_t ti = (ty * NES_WIDTH) + tx;

            if (bg && pix == 0x0) {
                uint32_t palette_bg = emu.memory.ppu_mem.palette[0x00];
                emu.back_buffer[ti] = MFB_RGB(palette_id_to_red(palette_bg), palette_id_to_green(palette_bg), palette_id_to_blue(palette_bg));
            } else if (palette_set == 0x0) {
                emu.back_buffer[ti] = MFB_RGB(gray_palette[pix*3], gray_palette[pix*3], gray_palette[pix*3]);
            } else {
                emu.back_buffer[ti] = MFB_RGB(palette_id_to_red(palette_set[pix-1]), palette_id_to_green(palette_set[pix-1]), palette_id_to_blue(palette_set[pix-1]));
            }
        }
    }
}

static void blit_chr_nt(emu_t& emu, uint32_t pix_x, uint32_t pix_y, uint32_t chr_index, bool bg, bool flip_x, bool flip_y, uint8_t* palette_set)
{
    uint8_t chr[8*8]; // 8x8 pixels is one character (tile)
    ppu_get_chr(emu, chr_index*16, chr, bg);

    for (uint32_t y = 0; y < 8; ++y)
    {
        for (uint32_t x = 0; x < 8; ++x)
        {
            uint8_t pix = chr[y*8+x];

            if (!bg && pix == 0x0)
                continue;
                
            uint32_t tx = pix_x + (flip_x ? 7 - x : x);
            uint32_t ty = pix_y + (flip_y ? 7 - y : y);

            uint32_t ti = (ty * NES_WIDTH * 2) + tx;

            if (bg && pix == 0x0) {
                uint32_t palette_bg = emu.memory.ppu_mem.palette[0x00];
                nt_window_buffer[ti] = MFB_RGB(palette_id_to_red(palette_bg), palette_id_to_green(palette_bg), palette_id_to_blue(palette_bg));
            } else if (palette_set == 0x0) {
                nt_window_buffer[ti] = MFB_RGB(gray_palette[pix*3], gray_palette[pix*3], gray_palette[pix*3]);
            } else {
                nt_window_buffer[ti] = MFB_RGB(palette_id_to_red(palette_set[pix-1]), palette_id_to_green(palette_set[pix-1]), palette_id_to_blue(palette_set[pix-1]));
            }
        }
    }
}

void dump_sprites(emu_t &emu)
{
    bool is_8x16 = ((emu.ppu.regs.PPUCTRL >> 5) & 0b1) == 0b1;

    static uint8_t palette_set[3];
    for (uint32_t sprite_i = 0; sprite_i < 64; ++sprite_i)
    {
        uint8_t sprite_data0 = emu.memory.ppu_mem.oam.arr2d[sprite_i][0]; // y
        uint8_t sprite_data1 = emu.memory.ppu_mem.oam.arr2d[sprite_i][1]; // tile index
        uint8_t sprite_data2 = emu.memory.ppu_mem.oam.arr2d[sprite_i][2]; // attributes
        uint8_t sprite_data3 = emu.memory.ppu_mem.oam.arr2d[sprite_i][3]; // x

        bool flip_x = !!(sprite_data2 & (1 << 6));
        bool flip_y = !!(sprite_data2 & (1 << 7));
        uint8_t palette_id = sprite_data2 & 0x3;

        // uint8_t palette_ids = emu.ppu.palette[0x11+palette_id*3];
        palette_set[0] = emu.memory.ppu_mem.palette[0x11+palette_id*4];
        palette_set[1] = emu.memory.ppu_mem.palette[0x12+palette_id*4];
        palette_set[2] = emu.memory.ppu_mem.palette[0x13+palette_id*4];

        blit_chr(emu, sprite_data3+2, sprite_data0, sprite_data1, false, flip_x, flip_y, palette_set);

        if (is_8x16) {
            blit_chr(emu, sprite_data3+2, sprite_data0+8, sprite_data1+1, false, flip_x, flip_y, palette_set);
        }
    }
}

void dump_ppu_vram(emu_t& emu)
{
    uint8_t palette_set[3];
    for (uint32_t yi = 0; yi < 30; ++yi)
    {
        for (uint32_t xi = 0; xi < 32; ++xi)
        {

            uint32_t nti = yi*32+xi;
            if (nti >= 0x800) {
                LOG_E("trying to access vram out of bounds!\n");
            }
            uint8_t chr_i = emu.memory.ppu_mem.vram[nti];

            // find correct attribute
            unsigned short attribute_addr = ((yi / 4)*8)+(xi / 4);
            uint8_t attribute = emu.memory.ppu_mem.vram[attribute_addr+0x3C0];

            uint32_t sub_x = (xi / 2) % 2;
            uint32_t sub_y = (yi / 2) % 2;
            uint32_t palette_id = 0;

            if (sub_x == 0 && sub_y == 0) {
                palette_id = attribute & 0x3;
            } else if (sub_x == 1 && sub_y == 0) {
                palette_id = (attribute >> 2) & 0x3;
            } else if (sub_x == 0 && sub_y == 1) {
                palette_id = (attribute >> 4) & 0x3;
            } else if (sub_x == 1 && sub_y == 1) {
                palette_id = (attribute >> 6) & 0x3;
            }
            palette_set[0] = emu.memory.ppu_mem.palette[0x01+palette_id*4];
            palette_set[1] = emu.memory.ppu_mem.palette[0x02+palette_id*4];
            palette_set[2] = emu.memory.ppu_mem.palette[0x03+palette_id*4];
            blit_chr(emu, xi*8, yi*8, chr_i, true, false, false, palette_set); // ignore fliping for now
        }
    }
}

void dump_nametables(emu_t &emu)
{
    uint8_t palette_set[3];
    uint16_t base_nt_addr = 0x2000;
    for (uint8_t nt_i = 0x0; nt_i < 4; nt_i++)
    {
        uint8_t nt_x = nt_i % 2;
        uint8_t nt_y = nt_i / 2;
        for (uint32_t yi = 0; yi < 30; ++yi)
        {
            for (uint32_t xi = 0; xi < 32; ++xi)
            {

                uint32_t nti = yi*32+xi;
                if (nti >= 0x800) {
                    LOG_E("trying to access vram out of bounds!");
                }
                
                uint8_t chr_i = emu.memory.ppu_memory_read( base_nt_addr+nti, true );

                // find correct attribute
                unsigned short attribute_addr = ((yi / 4)*8)+(xi / 4);
                uint8_t attribute = emu.memory.ppu_memory_read( base_nt_addr+attribute_addr+0x3C0, true );

                uint32_t sub_x = (xi / 2) % 2;
                uint32_t sub_y = (yi / 2) % 2;
                uint32_t palette_id = 0;

                if (sub_x == 0 && sub_y == 0) {
                    palette_id = attribute & 0x3;
                } else if (sub_x == 1 && sub_y == 0) {
                    palette_id = (attribute >> 2) & 0x3;
                } else if (sub_x == 0 && sub_y == 1) {
                    palette_id = (attribute >> 4) & 0x3;
                } else if (sub_x == 1 && sub_y == 1) {
                    palette_id = (attribute >> 6) & 0x3;
                }
                
                palette_set[0] = emu.memory.ppu_mem.palette[0x01+palette_id*4];
                palette_set[1] = emu.memory.ppu_mem.palette[0x02+palette_id*4];
                palette_set[2] = emu.memory.ppu_mem.palette[0x03+palette_id*4];
                
                blit_chr_nt(emu, xi*8 + nt_x*NES_WIDTH, yi*8 + nt_y*NES_HEIGHT, chr_i, true, false, false, palette_set); // ignore fliping for now
            }
        }
        base_nt_addr+=0x400;
    }
}

void clear_framebuffer(uint32_t* framebuffer, uint8_t r, uint8_t g, uint8_t b)
{
    for (uint32_t i = 0; i < NES_WIDTH * NES_HEIGHT; i++)
    {
        framebuffer[i] = MFB_RGB(r, g, b);
    }
}

void draw_glyph(uint32_t* framebuffer, uint32_t x, uint32_t y, uint8_t glyph)
{
    for (uint8_t gy = 0; gy < 8; ++gy)
    {
        for (uint8_t gx = 0; gx < 8; ++gx)
        {
            uint32_t bi = (y+gy)*NES_WIDTH+(x+gx);
            if (((font8x8_basic[glyph][gy]) & (1 << gx)) > 0)
            {
                framebuffer[bi] = MFB_RGB(255, 255, 255);
            }
        }
    }
}

void draw_text(uint32_t* framebuffer, uint32_t x, uint32_t y, const char* text, ...)
{

    static char _buffer[2048];
    static va_list va;

    va_start(va, text);
    vsnprintf(_buffer, 2048, text, va);
    va_end(va);

    char* glyphs = _buffer;
    while (*glyphs != 0x0)
    {
        draw_glyph(framebuffer, x, y, *glyphs);
        x+=8;
        glyphs++;
    }
}

} // nes


