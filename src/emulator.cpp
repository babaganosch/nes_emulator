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
    memory.init( rom );
    cpu.init( &cpu_clock_callback, memory );
    ppu.init( memory );
}

RESULT emu_t::step_cycles(int32_t cycles)
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