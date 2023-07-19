#include "nes.hpp"

namespace nes
{

static void cpu_callback(void * cookie)
{
    //printf("callback!\n");
}

RESULT emu_t::init(ines_rom_t &rom)
{
    memory.init();
    cpu.init(&cpu_callback, memory);

    // Map PRG ROM


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
    return RESULT_ERROR;
}


} // nes