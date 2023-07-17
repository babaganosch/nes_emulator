#include <cstdio>

#include "nes.hpp"

int main()
{
    printf("--- Enter Main ---\n");

    nes::mem_t memory{};
    nes::cpu_t cpu{};
    nes::ines_rom_t rom{};
    
    memory.init();
    cpu.init();

    cpu.execute();

    nes::load_rom_from_file("../data/nestest.nes", rom);
    rom.~ines_rom_t();

    printf("--- Shutting Down ---\n");
    
    return 0;
}