#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

square_oscillator_t::square_oscillator_t()
{
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = DEVICE_FORMAT;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &wave;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        LOG_E("Failed to open playback device.");
        throw;
    }

    waveConfig = ma_pulsewave_config_init(device.playback.format, device.playback.channels, device.sampleRate, 0.5, 0.0, 0);
    ma_pulsewave_init(&waveConfig, &wave);

    if (ma_device_start(&device) != MA_SUCCESS) {
        LOG_E("Failed to start playback device.");
        ma_device_uninit(&device);
        throw;
    }
}

square_oscillator_t::~square_oscillator_t()
{
    ma_device_uninit(&device);
    ma_pulsewave_uninit(&wave);
}

void square_oscillator_t::change_duty( double duty )
{
    ma_pulsewave_set_duty_cycle( &wave, duty );
}

void square_oscillator_t::change_volume( double volume )
{
    volume = volume > 1.0 ? 1.0 : volume;
    ma_pulsewave_set_amplitude( &wave, volume );
}

void square_oscillator_t::change_frequency( double frequency, bool reset_phase )
{
    if (reset_phase) ma_pulsewave_seek_to_pcm_frame( &wave, 0 );
    ma_pulsewave_set_frequency( &wave, frequency);
}

void apu_t::pulse_t::write( uint16_t address, uint8_t value )
{
    switch ( address % 0x4 )
    {
        case ( 0x0 ): 
        { // Envelope
            envelope.data = value;
            switch (value >> 6) {
                case 0: oscillator->change_duty( 12.5 / 100.0 ); break;
                case 1: oscillator->change_duty( 25.0 / 100.0 ); break;
                case 2: oscillator->change_duty( 50.0 / 100.0 ); break;
                case 3: oscillator->change_duty( 75.0 / 100.0 ); break;
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
            period = (length_counter_load.timer_high << 8) | timer_low;
            float freq = CPU_FREQ_NTCS / (16.0 * (period+1));
            oscillator->change_frequency( freq, false );
        } break;
        case ( 0x3 ): 
        { // Length counter load
            length_counter_load.data = value;
            period = (length_counter_load.timer_high << 8) | timer_low;
            length_counter_tmp = length_counter_lut[length_counter_load.load];
            float freq = CPU_FREQ_NTCS / (16.0 * (period+1));
            oscillator->change_frequency( freq, true );
            start_flag = true;
        } break;
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
    if (sweep_divider == 0 && sweep.shift > 0)
    {
        uint16_t change_amount = period >> sweep.shift;
        
        if (sweep.negate) {
            change_amount = ~change_amount;
            change_amount += two_compliment ? 1 : 0;
            change_amount &= 0x7FF;
        }
        period += change_amount;
        period &= 0x7FF;
    }

    // Update the period
    // 1.
    if (sweep_divider == 0 && sweep.enable && sweep.shift > 0)
    {
        if (period >= 8 && period <= 0x7FF)
        { // Should set period and not mute
            float freq = CPU_FREQ_NTCS / (16.0 * (period+1));
            oscillator->change_frequency( freq, false );
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
    if (!start_flag)
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
    } else
    {
        start_flag = false;
        envelope_decay = 15;
        envelope_divider = envelope.volume;
    }

    // Move to mixer.
    if (muted || length_counter == 0 || period < 8 || period > 0x7FF)
    {
        oscillator->change_volume( 0 );
    } else if (envelope.constant_volume == 1)
    { // Constant volume
        oscillator->change_volume( envelope.volume / 15.0 );
    } else
    { // Envelope controlled volume
        oscillator->change_volume( envelope_decay / 15.0 );
    }
}



} // nes