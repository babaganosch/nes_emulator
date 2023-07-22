#include "nes.hpp"

namespace nes
{

namespace
{
constexpr uint32_t PRG_PAGE_SIZE = 16 * 1024;
//constexpr uint32_t CHR_PAGE_SIZE = 8 * 1024;

emu_t* emulator_ref;
void cpu_clock_callback(void *cookie)
{
    emulator_ref->ppu.execute();
}

} // anonymous

RESULT emu_t::init(ines_rom_t &rom)
{
    emulator_ref = this;
    memory.init();
    cpu.init(&cpu_clock_callback, memory);
    ppu.init();

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