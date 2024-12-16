#include "nes.hpp"
#include "logging.hpp"
#include "mappers.hpp"

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

    mappers_lut[0]   = new mapper_nrom_t();
    mappers_lut[1]   = new mapper_mmc1_t();
    mappers_lut[2]   = new mapper_uxrom_t();
    mappers_lut[94]  = new mapper_un1rom_t();
    mappers_lut[180] = new mapper_unrom_configured_t();

    uint8_t mappers_instantiated = 0;
    for (uint8_t i = 0; i < 255; ++i) {
        if (mappers_lut[i]) ++mappers_instantiated;
    }
    LOG_D("Emulator instantiated %u mappers", mappers_instantiated);

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