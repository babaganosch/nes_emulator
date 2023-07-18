#include "nes.hpp"

#include <fstream>

namespace nes
{

namespace
{
const char INES_MAGIC[4] = { 'N', 'E', 'S', 0x1A };
const uint32_t PRG_PAGE_SIZE = 16 * 1024;
const uint32_t CHR_PAGE_SIZE = 8 * 1024;
const uint32_t INES_HEADER_SIZE = 16u;
} // anonymous

ines_rom_t::~ines_rom_t()
{
    clear_contents();
}

void ines_rom_t::clear_contents()
{
    if (prg_pages)
    {
        for (auto i = 0; i < header.prg_size; ++i)
        {
            delete prg_pages[i];
            prg_pages[i] = nullptr;
        }
    }
    delete prg_pages;
    prg_pages = nullptr;

    if (chr_pages)
    {
        for (auto i = 0; i < header.chr_size; ++i)
        {
            delete chr_pages[i];
            chr_pages = nullptr;
        }
    }
    delete chr_pages;
    chr_pages = nullptr;

    memset(&header, 0, INES_HEADER_SIZE);
}

RESULT ines_rom_t::load_from_file(const char* filepath)
{
    std::ifstream file;
    file.open(filepath, std::ios::in | std::ios::binary | std::ios::ate );
    const uint32_t file_size = file.tellg();
    file.seekg(0, file.beg);
    
    if (!file.good() || file_size == 0)
    {
        printf("Failed to open '%s'\n", filepath); // TODO(xxx): Proper logging;
        return RESULT_ERROR;
    }

    uint8_t* data = (uint8_t*)malloc(file_size * sizeof(uint8_t));
    file.read((char*)data, file_size);
    file.close();

    load_from_data(data, file_size);
    free(data);

    printf("ROM '%s' (%u bytes) loaded successfully.\n", filepath, file_size); // TODO(xxx): Proper logging;
    return RESULT_OK;
}

RESULT ines_rom_t::load_from_data(const uint8_t* data, const uint32_t size)
{
    if (size < INES_HEADER_SIZE)
    {
        printf("Size too small to contain iNES header.\n"); // TODO(xxx): Proper logging;
        return RESULT_INVALID_INES_HEADER;
    }

    // Initialize ROM
    clear_contents();

    // Copy iNES header
    memcpy(&header, data, INES_HEADER_SIZE);

    if (strncmp((const char*)header.magic, INES_MAGIC, 4) != 0)
    {
        printf("iNES header magic not valid.\n"); // TODO(xxx): Proper logging;
        return RESULT_INVALID_INES_HEADER;
    }

    prg_pages = new uint8_t*[header.prg_size];
    chr_pages = new uint8_t*[header.chr_size];

    // Copy PRG pages
    const uint8_t* data_ptr = &data[INES_HEADER_SIZE];
    for (auto i = 0; i < header.prg_size; ++i)
    {
        prg_pages[i] = new u_int8_t[PRG_PAGE_SIZE];
        memcpy(prg_pages[i], data_ptr, PRG_PAGE_SIZE);
        data_ptr += PRG_PAGE_SIZE;
    }

    // Copy CHR pages
    for (auto i = 0; i < header.chr_size; ++i)
    {
        chr_pages[i] = new uint8_t[CHR_PAGE_SIZE];
        memcpy(chr_pages[i], data_ptr, CHR_PAGE_SIZE);
        data_ptr += CHR_PAGE_SIZE;
    }

    const auto loaded_data_size = data_ptr - &data[INES_HEADER_SIZE];
    const auto expected_data_size = size - INES_HEADER_SIZE;
    if (loaded_data_size != expected_data_size)
    {
        printf("Error, written data not the same as specified.\n"); // TODO(xxx): Proper logging;
        return RESULT_ERROR;
    }

    return RESULT_OK;
}



} // nes