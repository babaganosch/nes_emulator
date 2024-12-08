#include "nes.hpp"
#include "logging.hpp"

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

void emu_t::init(ines_rom_t &rom)
{
    emulator_ref = this;
    cpu.init(&cpu_clock_callback, memory);
    ppu.init(memory);
    memory.init( rom );
    bool error = false;

    // Mirroring
    LOG_D("iNES header.flags_6: 0x%02X", rom.header.flags_6);
    LOG_D("iNES header.flags_7: 0x%02X", rom.header.flags_7);
    if (BIT_CHECK_HI(rom.header.flags_6, 0))
    {
        memory.ppu_mem.nt_mirroring = ppu_mem_t::nametable_mirroring::vertical;
        LOG_D("Vertical mirroring (horizontal arrangement)");
    } else {
        LOG_D("Horizontal mirroring (vertical arrangement)");
    }

    // Persistent memory on cartridge
    if (BIT_CHECK_HI(rom.header.flags_6, 1))
    {
        LOG_W("Cartridge contains some kind of persistent memory (unimplemented)");
    }

    // Mapper
    const uint8_t mapper = (rom.header.flags_7 & 0xF0) | ((rom.header.flags_6 & 0xF0) >> 4);
    if (mapper > 0x00)
    {
        LOG_W("Mapper: %u (unimplemented)", mapper);
    } else {
        LOG_D("Mapper: %u", mapper);
    }

    // Map PRG ROM
    if (rom.header.prg_size == 1) {
        memory.cartridge_mem.prg_lower_bank = memory.ines_rom->prg_pages[0];
        memory.cartridge_mem.prg_upper_bank = memory.ines_rom->prg_pages[0];
    } else if (rom.header.prg_size == 2) {
        memory.cartridge_mem.prg_lower_bank = memory.ines_rom->prg_pages[0];
        memory.cartridge_mem.prg_upper_bank = memory.ines_rom->prg_pages[1];
    } else {
        LOG_E("TODO: Solve mapping for more than two PRG ROM banks. PRG pages: %u encountered.", rom.header.prg_size);
        error = true;
    }

    // Map CHR ROM
    if (rom.header.chr_size == 1) {
        memory.cartridge_mem.chr_rom = memory.ines_rom->chr_pages[0];
    } else {
        LOG_E("TODO: Solve mapping for zero or more than one CHR ROM bank. CHR pages: %u encountered.", rom.header.chr_size);
        error = true;
    }

    // Try to grab the interrupt vectors
    cpu.vectors.NMI = cpu.peek_short( 0xFFFA );
    cpu.vectors.RESET = cpu.peek_short( 0xFFFC );
    cpu.vectors.IRQBRK = cpu.peek_short( 0xFFFE );

    // Reset program counter to reset vector
    cpu.regs.PC = cpu.vectors.RESET;

    if (error) throw RESULT_ERROR;
}

RESULT emu_t::step(int32_t cycles)
{
    while (cycles > 0)
    {
        cycles -= cpu.execute();
    }
    return RESULT_OK;
}

uint16_t emu_t::step_vblank()
{
    uint16_t extra_cycles_executed = 0;
    while (true)
    {
        bool start_in_vblank = ppu.render_state == ppu_t::render_states::vertical_blanking_line;
        extra_cycles_executed += cpu.execute();
        bool end_in_vblank = ppu.render_state == ppu_t::render_states::vertical_blanking_line;
        if (start_in_vblank && !end_in_vblank) break;
    }
    return extra_cycles_executed;
}


} // nes