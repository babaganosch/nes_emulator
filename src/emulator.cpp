#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION

#include "nes.hpp"
#include "logging.hpp"
#include "mappers.hpp"
#include "audio.hpp"

namespace nes
{

namespace
{
emu_t* emulator_ref;
audio_t* audio_ref;

float speed = 1.0f;

void callback_execute_ppu(void *cookie)
{
    emulator_ref->ppu.execute();
}

void callback_execute_apu(void *cookie)
{
    float output = emulator_ref->apu.execute();
    audio_ref->buffer_data( output );
}

void audio_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count)
{
    (void)input;   /* Unused. */
    float* frames_out = (float*)output;
    audio_t::audio_data_t* data = (audio_t::audio_data_t*)device->pUserData;
    float amplitude = data->amplitude;

    void* buffer;    
    data->drift = ma_rb_pointer_distance(&data->ring_buffer);
    if (data->drift > DRIFT_CORRECTION_THRESHOLD) {
        ma_rb_seek_read(&data->ring_buffer, DRIFT_CORRECTION_SKIP);
        LOG_W("Audio lagging behind, skipping forward.");
    }

    size_t size_in_bytes = frame_count * sizeof(float);
    ma_rb_acquire_read(&data->ring_buffer, &size_in_bytes, &buffer);

    size_t ready_frames = size_in_bytes / sizeof(float);
    memcpy(data->tmp_buffer, buffer, size_in_bytes);
    ma_rb_commit_read(&data->ring_buffer, size_in_bytes);

    for (ma_uint32 frame = 0; frame < frame_count; ++frame)
    {
        if (frame < ready_frames)
        {
            amplitude = data->tmp_buffer[frame];
        }

        for (ma_uint32 channel = 0; channel < device->playback.channels; ++channel)
        {
            frames_out[frame*device->playback.channels + channel] = amplitude;
        }
    }
    
    data->amplitude = amplitude;
}

} // anonymous

void emu_t::init(ines_rom_t &rom)
{
    emulator_ref = this;

    mappers_lut[0]   = new mapper_nrom_t();
    mappers_lut[1]   = new mapper_mmc1b_t();
    mappers_lut[2]   = new mapper_uxrom_t();
    mappers_lut[94]  = new mapper_un1rom_t();
    mappers_lut[180] = new mapper_unrom_configured_t();

    uint8_t mappers_instantiated = 0;
    for (uint8_t i = 0; i < 255; ++i) {
        if (mappers_lut[i]) ++mappers_instantiated;
    }
    LOG_D("Emulator instantiated %u mappers", mappers_instantiated);

    if (audio_ref) delete audio_ref;
    audio_ref = new audio_t();
    LOG_D("Audio interface initiated");
    
    memory.init( rom );
    cpu.init( &callback_execute_ppu, &callback_execute_apu, memory );
    ppu.init( memory );
    apu.init( memory );
}

RESULT emu_t::step_cycles(int32_t cycles)
{
    speed = (float)cycles / 29780.0;
    while (cycles > 0)
    {
        cycles -= cpu.execute();
    }
    return RESULT_OK;
}

uint16_t emu_t::step_vblank()
{
    uint16_t cycles_executed = 0;
    while (true)
    {
        bool start_in_vblank = ppu.check_vblank();
        cycles_executed += cpu.execute();
        bool end_in_vblank = ppu.check_vblank();
        if (start_in_vblank && !end_in_vblank) break;
    }

    return cycles_executed;
}

void audio_t::buffer_data( float amplitude)
{
    storage[stored_data++] = amplitude;

    if (cycle_count++ > (float)CYCLES_PER_CB * speed)
    {
        sample_data();
        cycle_count = 0;
    }
}

audio_t::audio_t()
{
    if (ma_rb_init(DEVICE_SAMPLE_RATE * sizeof(float), NULL, NULL, &data.ring_buffer) != MA_SUCCESS)
    {
        LOG_E("Failed to initialize ring buffer.");
        throw;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = DEVICE_FORMAT;
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

audio_t::~audio_t()
{
    ma_rb_uninit(&data.ring_buffer);
    ma_device_uninit(&device);
}

void audio_t::sample_data()
{
    if (stored_data < FRAMES_PER_CB) return; // Not enough frames ready to be sampled

    const float sample_offset = (float)stored_data / (float)FRAMES_PER_CB;
    size_t size_in_bytes = FRAMES_PER_CB * sizeof(float);

    size_t j = 0;
    for (float i = 0.0; i < stored_data; i += sample_offset)
    {
        data.tmp_buffer[j++] = storage[(int)i];
    }
    stored_data = 0;

    ma_rb_acquire_write(&data.ring_buffer, &size_in_bytes, &buffer);
    memcpy(buffer, data.tmp_buffer, size_in_bytes);
    ma_rb_commit_write(&data.ring_buffer, size_in_bytes);

}


} // nes