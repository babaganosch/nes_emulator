#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION

#include "nes.hpp"
#include "logging.hpp"
#include <atomic>
#include <iostream>

namespace nes
{

const uint8_t triangle_levels[32] = {
    0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

const uint8_t length_counter_lut[32] = { 
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E, // 0x00 - 0x0F
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E  // 0x10 - 0x1F
};

//static float compensation = 1.0;
std::atomic_int cycles_since_last{0};
static int compensation_cycles = 3;

void audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    float* pFramesOut = (float*)pOutput;
    audio_interface_t::audio_data_t* data = (audio_interface_t::audio_data_t*)pDevice->pUserData;
    float amplitude = data->amplitude;

    void*  buffer;
    float* tmp_buffer = data->tmp_buffer;
    bool   freeze_frames = false;

    LOG_D("drift: %u", ma_rb_pointer_distance(&data->ring_buffer));
    std::cout << cycles_since_last << std::endl;
    cycles_since_last.store(0);

    ma_int32 drift = ma_rb_pointer_distance(&data->ring_buffer);
    if (drift > 7000)
    { // Correction of drift
        compensation_cycles = 4;
    } else if (drift < 4000)
    {
        compensation_cycles = 3;
    }

    size_t sizeInBytes = frameCount * sizeof(float);
    if (ma_rb_acquire_read(&data->ring_buffer, &sizeInBytes, &buffer) != MA_SUCCESS) {
        //freeze_frames = true;
    }
    size_t framesGot = sizeInBytes / sizeof(float);
    
    memcpy(tmp_buffer, buffer, sizeInBytes);

    ma_result res = ma_rb_commit_read(&data->ring_buffer, sizeInBytes);
    if (res != MA_SUCCESS) {
        //freeze_frames = true;
    }

    for (ma_uint32 frame = 0; frame < frameCount; ++frame)
    {
        if (!freeze_frames && (frame < framesGot))
        {
            amplitude = tmp_buffer[frame];
        }

        for (ma_uint32 iChannel = 0; iChannel < pDevice->playback.channels; ++iChannel)
        {
            pFramesOut[frame*pDevice->playback.channels + iChannel] = amplitude;
        }
    }

    // Store current amplitude so it's possible to "freeze" future frames if needed
    data->amplitude = amplitude;

    (void)pInput;   /* Unused. */
}

audio_interface_t::audio_interface_t()
{
    size_t size = DEVICE_SAMPLE_RATE * sizeof(float);
    ma_rb_init(size, NULL, NULL, &data.ring_buffer);

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = DEVICE_CHANNELS;
    deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback      = audio_callback;
    deviceConfig.pUserData         = &data;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        LOG_E("Failed to open playback device.");
        throw;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        LOG_E("Failed to start playback device.");
        ma_device_uninit(&device);
        throw;
    }
}

audio_interface_t::~audio_interface_t()
{
    ma_rb_uninit(&data.ring_buffer);
}

static void* buffer;
void audio_interface_t::load( float value )
{
    // Is this a joke?
    size_t sizeInBytes = sizeof(float);
    ma_rb_acquire_write(&data.ring_buffer, &sizeInBytes, &buffer);
    memcpy(buffer, &value, sizeInBytes);
    ma_rb_commit_write(&data.ring_buffer, sizeInBytes);
}

apu_t::~apu_t()
{ // Shutdown
    if (audio) delete audio;
}

void apu_t::init(mem_t &mem)
{
    memory = &mem;
    memory->apu = this;

    if (!audio) audio = new audio_interface_t();
    
    cycle = 8; // Power up shenanigans
    LOG_I("APU initiated successfully");

}

static bool extra = true;
static int count = 0;
void apu_t::mixer()
{
    // Linear approximation
    float pulse_out = 0.00752 * (pulse_1.amplitude + pulse_2.amplitude);
    float tnd_out = 0.00851 * triangle_levels[triangle.period_index];
    float output = pulse_out + tnd_out;

    if (audio_sample_timer >= 38+extra) 
    { // Store a sample
        audio_sample_timer = 0;
        extra = true;
        if (count++ == compensation_cycles) {
            extra = false;
            count = 0;
        }
        audio->load(output);
    }

}

void apu_t::quarter_frame()
{
    pulse_1.tick_envelope();
    pulse_2.tick_envelope();
    triangle.tick_linear_counter();
}

void apu_t::half_frame()
{
    pulse_1.tick_sweep( false );
    pulse_2.tick_sweep( true );
    pulse_1.tick_length_counter();
    pulse_2.tick_length_counter();
    triangle.tick_length_counter();
}

void apu_t::execute()
{
    // Run the sequencer
    // NTSC timings
    uint16_t clock = cycle++;
    audio_sample_timer += 1.0;
    cycles_since_last++;

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

    if (pulse_1.length_counter_tmp > 0 && !pulse_1.muted) { pulse_1.length_counter = pulse_1.length_counter_tmp; } pulse_1.length_counter_tmp = 0;
    if (pulse_2.length_counter_tmp > 0 && !pulse_2.muted) { pulse_2.length_counter = pulse_2.length_counter_tmp; } pulse_2.length_counter_tmp = 0;
    if (triangle.length_counter_tmp > 0 && !triangle.muted) { triangle.length_counter = triangle.length_counter_tmp; } triangle.length_counter_tmp = 0;
    memory->cpu->irq_pending = frame_interrupt;

    // Tick oscillators
    if (cycle % 2) {
        pulse_1.tick();
        pulse_2.tick();
    }
    triangle.tick();
    // pulse.tick();
    // dmc.tick();

    mixer();
    
}


} // nes