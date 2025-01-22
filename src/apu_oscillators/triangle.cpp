#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

void apu_t::triangle_t::write( uint16_t address, uint8_t value )
{
    switch ( address )
    {
        case ( 0x4008 ): 
        { // Linear counter setup
            linear_counter_load.data = value;
        } break;
        case ( 0x4009 ): 
        { // Unused
            LOG_W("Write to 0x4009 (Triangle Unused)");
        } break;
        case ( 0x400A ): 
        { // Timer low
            timer_low = value;
            period = ((length_counter_load.timer_high << 8) | timer_low) + 1; // +1?
        } break;
        case ( 0x400B ): 
        { // Length counter load
            length_counter_load.data = value;
            period = ((length_counter_load.timer_high << 8) | timer_low) + 1; // +1?
            length_counter_tmp = length_counter_lut[length_counter_load.load];
            linear_counter_reload = true;
        } break;
    }
}

void apu_t::triangle_t::tick()
{
    bool playing = !(muted || length_counter == 0 || linear_counter == 0);
    timer += playing ? 1 : 0;
    if (timer > period)
    { // Advance
        period_index = (period_index + 1) % 32;
        timer = 0;
    }
}

void apu_t::triangle_t::tick_length_counter()
{
    if (!muted && length_counter_halt == 0)
    {
        if (length_counter_tmp > 0 && length_counter > 0) length_counter_tmp = 0;
        if (length_counter > 0)
        {
            length_counter--;
        }
    }
}

void apu_t::triangle_t::tick_linear_counter()
{
    //// Linear
    if (linear_counter_reload) {
        linear_counter = linear_counter_load.counter_reload;
    } else {
        if (linear_counter > 0)
        {
            linear_counter--;
        }
    }

    if (length_counter_halt == 0) 
    { // Only clear the reload flag if not halt is active
        linear_counter_reload = false;
    }
}

} // nes