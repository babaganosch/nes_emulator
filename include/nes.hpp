#ifndef NES_H
#define NES_H

#include <cstdint>
#include <vector>
#include <memory>

namespace nes
{

enum RESULT
{
    RESULT_FILE_NOT_FOUND = -1,
    RESULT_FILE_NOT_INES_ROM = -2,

    RESULT_ERROR = 0,

    RESULT_OK    = 1
};

struct cpu_t
{
    
    void init();
    void execute();
};

struct mem_t
{
    uint8_t data[0xFFFF];

    void init();
};

struct ines_rom_t
{
    struct header_t
    { // 16 bytes
        uint8_t magic[4];   // ASCII "NES" followed by MS-DOS end-of-file
        uint8_t prg_size;   // Size of PRG ROM in 16 KB units (16384 * x bytes)
        uint8_t chr_size;   // Size of CHR ROM in 8 KB units (8192 * y bytes) (value 0 means the board uses CHR RAM)
        uint8_t flags_6;    // Mapper, mirroring, battery, trainer
        uint8_t flags_7;    // Mapper, VS/Playchoice, NES 2.0
        uint8_t flags_8;    // PRG-RAM size (rarely used extension)
        uint8_t flags_9;    // TV system (rarely used extension)
        uint8_t flags_10;   // TV system, PRG-RAM presence (unofficial, rarely used extension)
        uint8_t padding[5]; // Unused padding
    } header;

    // Data is stored in pages, each page 16KB (16 * 1024 * x bytes)
    uint8_t** prg_pages{nullptr};
    uint8_t** chr_pages{nullptr};

    ~ines_rom_t();
};

RESULT load_rom_from_file(const char* filepath, ines_rom_t &rom);
RESULT load_rom_from_data(const uint8_t* data, const uint32_t size, ines_rom_t &rom);

} // nes

#endif /* NES_H */