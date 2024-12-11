#include "mappers.hpp"
#include "logging.hpp"
#include <memory>

namespace nes
{

mapper_t* mappers_lut[256] = {
    new mapper_nrom_t(),        // 0
    nullptr,                    // 1
    new mapper_uxrom_t(),       // 2
    nullptr,                    // 3
    nullptr,                    // 4
    nullptr,                    // 5
    nullptr,                    // 6
    nullptr,                    // 7
    nullptr,                    // 8
    nullptr,                    // 9
    nullptr,                    // 10
    nullptr,                    // 11
    nullptr,                    // 12
    nullptr,                    // 13
    nullptr,                    // 14
    nullptr,                    // 15
    nullptr,                    // 16
    nullptr,                    // 17
    nullptr,                    // 18
    nullptr,                    // 19
    nullptr,                    // 20
    nullptr,                    // 21
    nullptr,                    // 22
    nullptr,                    // 23
    nullptr,                    // 24
    nullptr,                    // 25
    nullptr,                    // 26
    nullptr,                    // 27
    nullptr,                    // 28
    nullptr,                    // 29
    nullptr,                    // 30
    nullptr,                    // 31
    nullptr,                    // 32
    nullptr,                    // 33
    nullptr,                    // 34
    nullptr,                    // 35
    nullptr,                    // 36
    nullptr,                    // 37
    nullptr,                    // 38
    nullptr,                    // 39
    nullptr,                    // 40
    nullptr,                    // 41
    nullptr,                    // 42
    nullptr,                    // 43
    nullptr,                    // 44
    nullptr,                    // 45
    nullptr,                    // 46
    nullptr,                    // 47
    nullptr,                    // 48
    nullptr,                    // 49
    nullptr,                    // 50
    nullptr,                    // 51
    nullptr,                    // 52
    nullptr,                    // 53
    nullptr,                    // 54
    nullptr,                    // 55
    nullptr,                    // 56
    nullptr,                    // 57
    nullptr,                    // 58
    nullptr,                    // 59
    nullptr,                    // 60
    nullptr,                    // 61
    nullptr,                    // 62
    nullptr,                    // 63
    nullptr,                    // 64
    nullptr,                    // 65
    nullptr,                    // 66
    nullptr,                    // 67
    nullptr,                    // 68
    nullptr,                    // 69
    nullptr,                    // 70
    nullptr,                    // 71
    nullptr,                    // 72
    nullptr,                    // 73
    nullptr,                    // 74
    nullptr,                    // 75
    nullptr,                    // 76
    nullptr,                    // 77
    nullptr,                    // 78
    nullptr,                    // 79
    nullptr,                    // 80
    nullptr,                    // 81
    nullptr,                    // 82
    nullptr,                    // 83
    nullptr,                    // 84
    nullptr,                    // 85
    nullptr,                    // 86
    nullptr,                    // 87
    nullptr,                    // 88
    nullptr,                    // 89
    nullptr,                    // 90
    nullptr,                    // 91
    nullptr,                    // 92
    nullptr,                    // 93
    nullptr,                    // 94
    nullptr,                    // 95
    nullptr,                    // 96
    nullptr,                    // 97
    nullptr,                    // 98
    nullptr,                    // 99
    nullptr,                    // 100
    nullptr,                    // 101
    nullptr,                    // 102
    nullptr,                    // 103
    nullptr,                    // 104
    nullptr,                    // 105
    nullptr,                    // 106
    nullptr,                    // 107
    nullptr,                    // 108
    nullptr,                    // 109
    nullptr,                    // 110
    nullptr,                    // 111
    nullptr,                    // 112
    nullptr,                    // 113
    nullptr,                    // 114
    nullptr,                    // 115
    nullptr,                    // 116
    nullptr,                    // 117
    nullptr,                    // 118
    nullptr,                    // 119
    nullptr,                    // 120
    nullptr,                    // 121
    nullptr,                    // 122
    nullptr,                    // 123
    nullptr,                    // 124
    nullptr,                    // 125
    nullptr,                    // 126
    nullptr,                    // 127
    nullptr,                    // 128
    nullptr,                    // 129
    nullptr,                    // 130
    nullptr,                    // 131
    nullptr,                    // 132
    nullptr,                    // 133
    nullptr,                    // 134
    nullptr,                    // 135
    nullptr,                    // 136
    nullptr,                    // 137
    nullptr,                    // 138
    nullptr,                    // 139
    nullptr,                    // 140
    nullptr,                    // 141
    nullptr,                    // 142
    nullptr,                    // 143
    nullptr,                    // 144
    nullptr,                    // 145
    nullptr,                    // 146
    nullptr,                    // 147
    nullptr,                    // 148
    nullptr,                    // 149
    nullptr,                    // 150
    nullptr,                    // 151
    nullptr,                    // 152
    nullptr,                    // 153
    nullptr,                    // 154
    nullptr,                    // 155
    nullptr,                    // 156
    nullptr,                    // 157
    nullptr,                    // 158
    nullptr,                    // 159
    nullptr,                    // 160
    nullptr,                    // 161
    nullptr,                    // 162
    nullptr,                    // 163
    nullptr,                    // 164
    nullptr,                    // 165
    nullptr,                    // 166
    nullptr,                    // 167
    nullptr,                    // 168
    nullptr,                    // 169
    nullptr,                    // 170
    nullptr,                    // 171
    nullptr,                    // 172
    nullptr,                    // 173
    nullptr,                    // 174
    nullptr,                    // 175
    nullptr,                    // 176
    nullptr,                    // 177
    nullptr,                    // 178
    nullptr,                    // 179
    nullptr,                    // 180
    nullptr,                    // 181
    nullptr,                    // 182
    nullptr,                    // 183
    nullptr,                    // 184
    nullptr,                    // 185
    nullptr,                    // 186
    nullptr,                    // 187
    nullptr,                    // 188
    nullptr,                    // 189
    nullptr,                    // 190
    nullptr,                    // 191
    nullptr,                    // 192
    nullptr,                    // 193
    nullptr,                    // 194
    nullptr,                    // 195
    nullptr,                    // 196
    nullptr,                    // 197
    nullptr,                    // 198
    nullptr,                    // 199
    nullptr,                    // 200
    nullptr,                    // 201
    nullptr,                    // 202
    nullptr,                    // 203
    nullptr,                    // 204
    nullptr,                    // 205
    nullptr,                    // 206
    nullptr,                    // 207
    nullptr,                    // 208
    nullptr,                    // 209
    nullptr,                    // 210
    nullptr,                    // 211
    nullptr,                    // 212
    nullptr,                    // 213
    nullptr,                    // 214
    nullptr,                    // 215
    nullptr,                    // 216
    nullptr,                    // 217
    nullptr,                    // 218
    nullptr,                    // 219
    nullptr,                    // 220
    nullptr,                    // 221
    nullptr,                    // 222
    nullptr,                    // 223
    nullptr,                    // 224
    nullptr,                    // 225
    nullptr,                    // 226
    nullptr,                    // 227
    nullptr,                    // 228
    nullptr,                    // 229
    nullptr,                    // 230
    nullptr,                    // 231
    nullptr,                    // 232
    nullptr,                    // 233
    nullptr,                    // 234
    nullptr,                    // 235
    nullptr,                    // 236
    nullptr,                    // 237
    nullptr,                    // 238
    nullptr,                    // 239
    nullptr,                    // 240
    nullptr,                    // 241
    nullptr,                    // 242
    nullptr,                    // 243
    nullptr,                    // 244
    nullptr,                    // 245
    nullptr,                    // 246
    nullptr,                    // 247
    nullptr,                    // 248
    nullptr,                    // 249
    nullptr,                    // 250
    nullptr,                    // 251
    nullptr,                    // 252
    nullptr,                    // 253
    nullptr,                    // 254
    nullptr                     // 255
};

//////// mapper basic behaviour
void mapper_t::init( mem_t* memory_ref ) {
    memory = memory_ref;
    uint8_t prg_banks = memory->ines_rom->header.prg_size;
    uint8_t chr_banks = memory->ines_rom->header.chr_size;

    LOG_D("PRG Banks: %u", prg_banks);
    LOG_D("CHR Banks: %u", chr_banks);

    // Map PRG ROM
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[0];
    memory->cartridge_mem.prg_upper_bank = memory->ines_rom->prg_pages[prg_banks-1];
    // Map CHR ROM/RAM
    if (chr_banks > 0) 
    { // ROM
        memory->cartridge_mem.chr_rom = memory->ines_rom->chr_pages[0];
    } else
    { // RAM
        memory->cartridge_mem.chr_mode = &memory->cartridge_mem.chr_ram;
        memory->cartridge_mem.chr_ram = new uint8_t[CHR_PAGE_SIZE];
    }
}

uint8_t mapper_t::cpu_read( uint16_t address ) {
    if ( address < 0x6000 ) return memory->cartridge_mem.expansion_rom[ address - 0x4020 ];
    if ( address < 0x8000 ) return memory->cartridge_mem.sram[ address  - 0x6000];
    if ( address < 0xC000 ) return memory->cartridge_mem.prg_lower_bank[ address - 0x8000 ];
    else return memory->cartridge_mem.prg_upper_bank[ address - 0xC000 ];
}

uint8_t mapper_t::ppu_read( uint16_t address ) {
    return memory->cartridge_mem.chr_rom[ address ];
}

void mapper_t::cpu_write( uint16_t address, uint8_t value ) {
    return;
}

void mapper_t::ppu_write( uint16_t address, uint8_t value ) {
    memory->cartridge_mem.chr_rom[ address ] = value;
}

//////// mapper 000 - NROM


//////// mapper 002 - UxROM
uint8_t mapper_uxrom_t::ppu_read( uint16_t address ) {
    return memory->cartridge_mem.chr_ram[ address ];
}

void mapper_uxrom_t::cpu_write( uint16_t address, uint8_t value ) {
    uint8_t bank = address & 0b00000111;
    memory->cartridge_mem.prg_lower_bank = memory->ines_rom->prg_pages[bank];
}

void mapper_uxrom_t::ppu_write( uint16_t address, uint8_t value ) {
    memory->cartridge_mem.chr_ram[ address ] = value;
}


} // nes