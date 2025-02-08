#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

namespace {

constexpr uint16_t period_lut[32] = { // Periods in CPU Cycles
    0x04, 0x08, 0x10, 0x20, 0x40, 0x60, 0x80, 0xA0, 0xCA, 0xFE, 0x17C, 0x1FC, 0x2FA, 0x3F8, 0x7F2, 0xFEC, // NTSC: 0x00 - 0x0F 
    0x04, 0x08, 0x0E, 0x1E, 0x3C, 0x58, 0x76, 0x94, 0xBC, 0xEC, 0x162, 0x1D8, 0x2C4, 0x3B0, 0x762, 0xEC2  // PAL:  0x10 - 0x1F
};

} // anonymous

void apu_t::noise_t::write( uint16_t address, uint8_t value )
{
    switch ( address )
    {
        case ( 0x400C ): 
        { // Envelope
            envelope.data = value;
        } break;
        case ( 0x400D ): 
        { // unused
            LOG_W("Write to 0x400D (Noise Unused)");
        } break;
        case ( 0x400E ): 
        { // Control
            control.data = value;
            period = period_lut[control.period]; // 0x0 - 0xF NTSC
        } break;
        case ( 0x400F ): 
        { // Length counter load
            length_counter_load.data = value;
            length_counter_tmp = length_counter_lut[length_counter_load.load];
            start_flag = true;
        } break;
    }
}

void apu_t::noise_t::tick()
{
    if (++timer > period)
    { // Advance
        uint8_t feedback = control.mode ? (shift & 0x0001) ^ ((shift & 0x0040) >> 6) : (shift & 0x0001) ^ ((shift & 0x0002) >> 1);
        shift = shift >> 1;
        shift |= feedback << 14;
        amplitude = feedback * volume;
        timer = 0;
    }
}

void apu_t::noise_t::tick_length_counter()
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

void apu_t::noise_t::tick_envelope()
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
    if (muted || length_counter == 0)
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