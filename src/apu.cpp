#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include "nes.hpp"
#include "logging.hpp"

namespace nes
{

#define CPU_FREQ_NTCS 1789772
#define CPU_FREQ_PAL  1662607

#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

const uint8_t length_counter_lut[] = { 
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E, // 0x00 - 0x0F
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E  // 0x10 - 0x1F
};

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_waveform* pWave;
    
    MA_ASSERT(pDevice->playback.channels == DEVICE_CHANNELS);

    pWave = (ma_waveform*)pDevice->pUserData;
    MA_ASSERT(pWave != NULL);

    ma_waveform_read_pcm_frames(pWave, pOutput, frameCount, NULL);

    (void)pInput;   /* Unused. */
}

oscillator_t::oscillator_t( ma_waveform_type type )
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

    waveConfig = ma_waveform_config_init(device.playback.format, device.playback.channels, device.sampleRate, type, 0.0, 220);

    ma_waveform_init(&waveConfig, &wave);

    if (ma_device_start(&device) != MA_SUCCESS) {
        LOG_E("Failed to start playback device.");
        ma_device_uninit(&device);
        throw;
    }
}

oscillator_t::~oscillator_t()
{
    ma_device_uninit(&device);
    ma_waveform_uninit(&wave);
}

void oscillator_t::change_volume( double volume )
{
    volume = volume > 1.0 ? 1.0 : volume;
    ma_waveform_set_amplitude( &wave, volume );
}

void oscillator_t::change_frequency( double frequency, bool reset_phase )
{
    if (reset_phase) ma_waveform_seek_to_pcm_frame( &wave, 0 );
    ma_waveform_set_frequency( &wave, frequency);
}

apu_t::~apu_t()
{ // Shutdown
    if (pulse_1.oscillator)  delete pulse_1.oscillator;
    if (pulse_2.oscillator)  delete pulse_2.oscillator;
    if (triangle) delete triangle;
    if (noise)    delete noise;
    if (dmc)      delete dmc;
}

void apu_t::pulse_t::write( uint16_t address, uint8_t value )
{
    switch ( address % 0x4 )
    {
        case ( 0x0 ): 
        { // Envelope
            envelope.data = value;
            volume_mode = (volume_mode_t)envelope.constant_volume;
            playback_mode = (playback_mode_t)envelope.length_counter_halt;
        } break;
        case ( 0x1 ): 
        { // Sweep
            sweep.data = value;
            sweep_reload = true;
        } break;
        case ( 0x2 ): 
        { // Timer low
            timer_low = value;
            uint16_t t = (length_counter_load.timer_high << 8) | timer_low;
            raw_period = t;
            float freq = CPU_FREQ_NTCS / (16.0 * (t+1));
            oscillator->change_frequency( freq, false );
        } break;
        case ( 0x3 ): 
        { // Length counter load
            length_counter_load.data = value;
            uint16_t t = (length_counter_load.timer_high << 8) | timer_low;
            raw_period = t;
            if (!muted) length_counter = length_counter_lut[length_counter_load.load]; // +1 to mute on zero?
            float freq = CPU_FREQ_NTCS / (16.0 * (t+1));
            oscillator->change_frequency( freq, true );
            start_flag = true;
        } break;
    }
}

void apu_t::pulse_t::tick_length_counter()
{
    if (!muted && length_counter > 0 && length_counter_halt == 0)
    {
        length_counter--;
    }
}

void apu_t::pulse_t::tick_sweep( bool two_compliment )
{
    // Calculate the period
    if (sweep_divider == 0 && sweep.shift > 0)
    {
        uint16_t change_amount = raw_period >> sweep.shift;
        
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
            float freq = CPU_FREQ_NTCS / (16.0 * (raw_period+1));
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
    if (start_flag)
    {
        envelope_divider = 15;
        start_flag = false;
    } else
    {
        if (envelope_divider > 0)
        {
            if (--envelope_divider == 0)
            {
                if (playback_mode == playback_mode_t::looping)
                {
                    envelope_divider = 15;
                } else
                {
                    envelope_divider = 0;
                }
            }
        }
    }

    // Move to mixer.
    if (muted || length_counter == 0 || raw_period < 8 || raw_period > 0x7FF)
    {
        oscillator->change_volume( 0 );
    } else if (volume_mode == volume_mode_t::constant)
    {
        oscillator->change_volume( envelope.volume / 15.0 );
    } else
    {
        oscillator->change_volume( envelope_divider / 15.0 );
    }
}

void apu_t::init(mem_t &mem)
{
    memory = &mem;
    memory->apu = this;
    
    if (!pulse_1.oscillator) pulse_1.oscillator = new oscillator_t( ma_waveform_type_square );
    if (!pulse_2.oscillator) pulse_2.oscillator = new oscillator_t( ma_waveform_type_square );
    
    cycle = 8; // Power up shenanigans
    LOG_I("APU initiated successfully");

}

void apu_t::quarter_frame()
{
    pulse_1.tick_envelope();
    pulse_2.tick_envelope();
}

void apu_t::half_frame()
{
    pulse_1.tick_sweep( false );
    pulse_2.tick_sweep( true );
    pulse_1.tick_length_counter();
    pulse_2.tick_length_counter();
}

void apu_t::execute()
{
    // Run the sequencer
    // NTSC timings

    // Phase reset, todo?
    //if (pulse_1.start_flag || pulse_2.start_flag) {
        //cycle = frame_counter.sequencer_mode == 0 ? 29828 : 37280;
    //}
    uint16_t clock = cycle++;

    if (reset_frame_counter > 0 && --reset_frame_counter == 0)
    {
        cycle = 0;
        clock = 0;
        if (frame_counter.sequencer_mode == 1) {
            quarter_frame();
            half_frame();
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
                memory->cpu->irq_pending = true;
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
}


} // nes