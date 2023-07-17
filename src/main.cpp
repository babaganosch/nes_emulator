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

    nes::load_rom_from_file("../data/nestest.nes", rom);

    cpu.execute();

    rom.~ines_rom_t();

    printf("mem size: %04lx\n", sizeof(memory));

    printf("%u %u %u\n", memory[0x0060], memory[0x0061], memory[0x0062]);
    memory[0x0060] = 0xA;
    memory[0x0061] = 0xB;
    memory[0x0062] = 0xC;
    printf("%u %u %u\n", memory[0x0060], memory[0x0061], memory[0x0062]);

    printf("--- Shutting Down ---\n");
    
    return 0;
}