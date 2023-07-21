#include "nes.hpp"

namespace nes
{

namespace
{
const uint32_t PRG_PAGE_SIZE = 16 * 1024;
//const uint32_t CHR_PAGE_SIZE = 8 * 1024;
} // anonymous

static void cpu_callback(void * cookie)
{
    //printf("callback!\n");
}

RESULT emu_t::init(ines_rom_t &rom)
{
    memory.init();
    cpu.init(&cpu_callback, memory);

    // Map PRG ROM
    if (rom.header.prg_size == 1) {
        memcpy(memory.prg_lower_bank, rom.prg_pages[0], PRG_PAGE_SIZE * sizeof(uint8_t));
        memcpy(memory.prg_upper_bank, rom.prg_pages[0], PRG_PAGE_SIZE * sizeof(uint8_t));
    } else if (rom.header.prg_size == 2) {
        memcpy(memory.prg_lower_bank, rom.prg_pages[0], PRG_PAGE_SIZE * sizeof(uint8_t));
        memcpy(memory.prg_upper_bank, rom.prg_pages[1], PRG_PAGE_SIZE * sizeof(uint8_t));
    } else {
        printf("TODO: Solve mapping for more than two PRG ROM bank.");
        return RESULT_ERROR;
    }

    // Map CHR ROM


    return RESULT_OK;
}

RESULT emu_t::step(uint16_t cycles)
{
    while (cycles > 0)
    {
        cpu.execute();
        cycles--;
    }
    return RESULT_OK;
}


} // nes