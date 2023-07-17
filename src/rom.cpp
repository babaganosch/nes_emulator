#include "nes.hpp"

#include <fstream>

namespace nes
{

namespace
{
static const char INES_MAGIC[4] = { 'N', 'E', 'S', 0x1A };
} // anonymous

ines_rom_t::~ines_rom_t()
{
    if (prg_pages)
    {
        for (auto i = 0; i < header.prg_size; ++i)
        {
            delete prg_pages[i];
        }
    }
    delete prg_pages;
    prg_pages = nullptr;

    if (chr_pages)
    {
        for (auto i = 0; i < header.chr_size; ++i)
        {
            delete chr_pages[i];
        }
    }
    delete chr_pages;
    chr_pages = nullptr;
}

RESULT load_rom_from_file(const char* filepath, ines_rom_t &rom)
{
    std::ifstream file;
    file.open(filepath, std::ios::in | std::ios::binary | std::ios::ate );
    const uint32_t file_size = file.tellg();
    file.seekg(0, file.beg);
    
    if (!file.good() || file_size == 0)
    {
        printf("Failed to open '%s'\n", filepath);
        return RESULT_ERROR;
    }

    uint8_t* data = (uint8_t*)malloc(file_size * sizeof(uint8_t));
    file.read((char*)data, file_size);
    file.close();

    load_rom_from_data(data, file_size, rom);

    free(data);
    return RESULT_OK;
}

RESULT load_rom_from_data(const uint8_t* data, const uint32_t size, ines_rom_t &rom)
{
    if (size < 16)
    {
        printf("Datasize of rom too small.\n");
        return RESULT_ERROR;
    }

    // Copy iNES header
    memcpy(&rom.header, data, 16);

    if (strncmp((const char*)rom.header.magic, INES_MAGIC, 4) != 0)
    {
        printf("iNES header magic not valid.\n"); // TODO(xxx): Proper logging;
        return RESULT_FILE_NOT_INES_ROM;
    }


    return RESULT_OK;
}



} // nes