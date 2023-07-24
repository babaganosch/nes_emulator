#include "nes.hpp"

namespace nes
{

namespace
{
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
        memory.cartridge_mem.prg_lower_bank = rom.prg_pages[0];
        memory.cartridge_mem.prg_upper_bank = rom.prg_pages[0];
    } else if (rom.header.prg_size == 2) {
        memory.cartridge_mem.prg_lower_bank = rom.prg_pages[0];
        memory.cartridge_mem.prg_upper_bank = rom.prg_pages[1];
    } else {
        printf("TODO: Solve mapping for more than two PRG ROM bank.");
        return RESULT_ERROR;
    }

    // Map CHR ROM

    // Try to grab the interrupt vectors
    cpu.vectors.NMI = cpu.peek_memory( 0xFFFA );
    cpu.vectors.RESET = cpu.peek_memory( 0xFFFC );
    cpu.vectors.IRQBRK = cpu.peek_memory( 0xFFFE );

    // Reset program counter to reset vector
    cpu.regs.PC = cpu.vectors.RESET;

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