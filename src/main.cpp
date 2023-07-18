#include <cstdio>

#include "nes.hpp"

namespace 
{
const char* nes_test_rom = "../data/nestest.nes";
}

int main()
{
    printf("--- Enter Main ---\n");
    nes::RESULT ret = nes::RESULT_OK;

    nes::ines_rom_t rom{};
    nes::emu_t emu{};

    rom.load_from_file(nes_test_rom);
    emu.init(rom);
    
    while (ret > 0)
    {
        ret = emu.step(3);
    }

    printf("--- Shutting Down ---\n");
    rom.~ines_rom_t();

    printf("Exiting with code %d\n", ret);
    return ret;
}