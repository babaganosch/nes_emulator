#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

triangle_oscillator_t::triangle_oscillator_t()
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

    waveConfig = ma_waveform_config_init(device.playback.format, device.playback.channels, device.sampleRate, ma_waveform_type_triangle, 1.0, 0);
    ma_waveform_init(&waveConfig, &wave);

    if (ma_device_start(&device) != MA_SUCCESS) {
        LOG_E("Failed to start playback device.");
        ma_device_uninit(&device);
        throw;
    }
}

triangle_oscillator_t::~triangle_oscillator_t()
{
    ma_device_uninit(&device);
    ma_waveform_uninit(&wave);
}

void triangle_oscillator_t::change_volume( double volume )
{
    volume = volume > 1.0 ? 1.0 : volume;
    ma_waveform_set_amplitude( &wave, volume );
}

void triangle_oscillator_t::change_frequency( double frequency )
{
    ma_waveform_set_frequency( &wave, frequency);
}

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
            period = (length_counter_load.timer_high << 8) | timer_low;
            float freq = CPU_FREQ_NTCS / (32.0 * (period+1));
            oscillator->change_frequency( freq );
        } break;
        case ( 0x400B ): 
        { // Length counter load
            length_counter_load.data = value;
            period = (length_counter_load.timer_high << 8) | timer_low;
            float freq = CPU_FREQ_NTCS / (32.0 * (period+1));
            oscillator->change_frequency( freq );
            length_counter_tmp = length_counter_lut[length_counter_load.load];
            linear_counter_reload = true;
        } break;
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

    // Move to mixer.
    //LOG_D("%u %u %u", muted, length_counter, linear_counter);
    if (muted || length_counter == 0 || linear_counter == 0)
    {
        ma_device_stop(&oscillator->device);
    } else
    {
        ma_device_start(&oscillator->device);
    }
}

} // nes