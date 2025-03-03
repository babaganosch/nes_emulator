#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

constexpr uint8_t length_counter_lut[32] = { // 0x00 - 0x1F
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E
};

void apu_t::init(mem_t* mem)
{
    memory = mem;
    memory->apu = this;
    dmc.memory_reader.memory = mem;
    
    cycle = 8; // Power up shenanigans
    LOG_I("APU initiated successfully");
}

void apu_t::mixer()
{
    // Linear approximation
    float pulse_out = 0.00752 * ((float)pulse_1.amplitude + (float)pulse_2.amplitude);
    float tnd_out = (0.00851 * (float)triangle.amplitude) + (0.00494 * (float)noise.amplitude) + (0.00335 * (float)dmc.output_level);
    output = pulse_out + tnd_out;

}

void apu_t::quarter_frame()
{
    pulse_1.tick_envelope();
    pulse_2.tick_envelope();
    triangle.tick_linear_counter();
    noise.tick_envelope();
}

void apu_t::half_frame()
{
    pulse_1.tick_sweep( false );
    pulse_2.tick_sweep( true );
    pulse_1.tick_length_counter();
    pulse_2.tick_length_counter();
    triangle.tick_length_counter();
    noise.tick_length_counter();
}

static bool irq_lag[3]{false};
static uint8_t irq_lag_index{0};
float apu_t::execute()
{
    // Run the sequencer
    // NTSC timings
    uint16_t clock = cycle++;

    // Actual IRQ seems to lag 2 cycles behind. It's probably wrong but it seems to work. 
    irq_lag[irq_lag_index++] = frame_interrupt || dmc.interrupt_flag;
    irq_lag_index %= 3;
    memory->cpu->irq_pending = irq_lag[irq_lag_index];

    if (reset_frame_counter > 0 && --reset_frame_counter == 0)
    {
        cycle = 0;
        clock = 0;
        if (frame_counter.sequencer_mode == 1) {
            half_frame();
            quarter_frame();
        }
    }

    if (frame_counter.sequencer_mode == 0)
    { // Four steps
        if (clock == 7456)
        { // 1
            quarter_frame();
        } else if (clock == 14912)
        { // 2
            half_frame();
            quarter_frame();
        } else if (clock == 22370)
        { // 3
            quarter_frame();
        } else if (clock == 29827)
        { // 4.1
            if (frame_counter.interrupt_inhibit == 0)
            {
                frame_interrupt = true;
            }
        } else if (clock == 29828)
        { // 4.2
            half_frame();
            quarter_frame();
            if (frame_counter.interrupt_inhibit == 0)
            {
                frame_interrupt = true;
            }
        } else if (clock >= 29829)
        { // 4.3
            cycle = 0;
            if (frame_counter.interrupt_inhibit == 0)
            {
                frame_interrupt = true;
            }
        }
    } else
    { // Five steps
        if (clock == 7456)
        { // 1
            quarter_frame();
        } else if (clock == 14912) // 912
        { // 2
            half_frame();
            quarter_frame();
        } else if (clock == 22370)
        { // 3
            quarter_frame();
        } 
        // 4 (empty)
        else if (clock == 37280)
        { // 5
            half_frame();
            quarter_frame();
        } else if (clock >= 37281) cycle = 0;
    }

    // TODO: Move in to some kind of post-cycle function (halt flag is set at the end of the tick?)
    pulse_1.length_counter_halt = pulse_1.envelope.length_counter_halt;
    pulse_2.length_counter_halt = pulse_2.envelope.length_counter_halt;
    triangle.length_counter_halt = triangle.linear_counter_load.control;
    noise.length_counter_halt = noise.envelope.length_counter_halt;

    if (pulse_1.length_counter_tmp > 0 && !pulse_1.muted) { pulse_1.length_counter = pulse_1.length_counter_tmp; } pulse_1.length_counter_tmp = 0;
    if (pulse_2.length_counter_tmp > 0 && !pulse_2.muted) { pulse_2.length_counter = pulse_2.length_counter_tmp; } pulse_2.length_counter_tmp = 0;
    if (triangle.length_counter_tmp > 0 && !triangle.muted) { triangle.length_counter = triangle.length_counter_tmp; } triangle.length_counter_tmp = 0;
    if (noise.length_counter_tmp > 0 && !noise.muted) { noise.length_counter = noise.length_counter_tmp; } noise.length_counter_tmp = 0;

    // Tick oscillators
    pulse_1.tick();
    pulse_2.tick();
    triangle.tick();
    noise.tick();
    dmc.tick();

    mixer();
    
    return output;
}


} // nes