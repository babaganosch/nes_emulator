#include "logging.hpp"
#include "nes.hpp"

namespace nes
{

void apu_t::pulse_t::write( uint16_t address, uint8_t value )
{
    switch ( address % 0x4 )
    {
        case ( 0x0 ): 
        { // Envelope
            envelope.data = value;
            switch (envelope.duty) {
                case 0: duty = 0b01000000; break; // 12.5 %
                case 1: duty = 0b01100000; break; // 25 %
                case 2: duty = 0b01111000; break; // 50 %
                case 3: duty = 0b10011111; break; // 25 % (inverted)
            }
        } break;
        case ( 0x1 ): 
        { // Sweep
            sweep.data = value;
            sweep_reload = true;
        } break;
        case ( 0x2 ): 
        { // Timer low
            timer_low = value;
            period = ((length_counter_load.timer_high << 8) | timer_low);
            period *= 2; // Convert from CPU cycles to APU cycles
        } break;
        case ( 0x3 ): 
        { // Length counter load
            length_counter_load.data = value;
            period = ((length_counter_load.timer_high << 8) | timer_low);
            period *= 2; // Convert from CPU cycles to APU cycles
            duty_index = 0;
            length_counter_tmp = length_counter_lut[length_counter_load.load];
            start_flag = true;
        } break;
    }
}

void apu_t::pulse_t::tick()
{
    if (++timer > period)
    { // Advance
        amplitude = (duty & (1 << (7-duty_index))) > 0 ? 1 : 0;
        amplitude *= volume;
        duty_index = (duty_index+1) % 8;
        timer = 0;
    }
}

void apu_t::pulse_t::tick_length_counter()
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

void apu_t::pulse_t::tick_sweep( bool two_compliment )
{
    // Calculate the period
    uint16_t raw_period = period;
    if (sweep_divider == 0 && sweep.shift > 0)
    {
        uint16_t change_amount = period >> sweep.shift;
        
        if (sweep.negate) {
            change_amount = ~change_amount;
            change_amount += two_compliment ? 1 : 0;
            change_amount &= 0x7FF;
        }
        raw_period += change_amount;
        raw_period &= 0x7FF;
    }

    // Update the period
    // 1.
    if (sweep_divider == 0 && sweep.enable && sweep.shift > 0)
    {
        if (raw_period >= 8 && raw_period <= 0x7FF)
        { // Should set period and not mute
            period = raw_period;
        }
    }
    
    // 2.
    if (sweep_divider == 0 || sweep_reload)
    {
        sweep_divider = sweep.divider;
        sweep_reload = false;
    } else
    {
        sweep_divider--;
    }
    
}

void apu_t::pulse_t::tick_envelope()
{
    if (start_flag)
    {
        start_flag = false;
        envelope_decay = 15;
        envelope_divider = envelope.volume;
    } else
    {
        if (envelope_divider > 0)
        { 
            envelope_divider--;
        } else 
        { // clocked at zero
            envelope_divider = envelope.volume;
            // decay is clocked
            if (envelope_decay > 0)
            {
                envelope_decay--;
            } else if (length_counter_halt == 1)
            { // looping, reload!
                envelope_decay = 15;
            }
        }
    }

    // Move to mixer.
    if (muted || length_counter == 0 || period < 8 || period > 0x7FF)
    {
        volume = 0;
    } else if (envelope.constant_volume == 1)
    { // Constant volume
        volume = envelope.volume;
    } else
    { // Envelope controlled volume
        volume = envelope_decay;
    }
}



} // nes