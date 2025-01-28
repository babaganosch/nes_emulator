#ifndef AUDIO_HPP
#define AUDIO_HPP
#include <miniaudio.h>

namespace nes
{

#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000
#define FRAMES_PER_CB       480
#define CYCLES_PER_CB       17864 // 10ms based upon 29781 per 16.67ms

#define DRIFT_CORRECTION_THRESHOLD      20000
#define DRIFT_CORRECTION_SKIP           12000 // 250ms

struct audio_t
{
    audio_t();
    ~audio_t();
    
    struct audio_data_t {
        ma_rb  ring_buffer;
        float  tmp_buffer[FRAMES_PER_CB];
        float  amplitude{0};
        size_t drift{0};
    } data;

    float  storage[DEVICE_SAMPLE_RATE];
    void*  buffer{nullptr};
    size_t stored_data{0};
    size_t cycle_count{0};

    void buffer_data( float amplitude );
    void sample_data();

    ma_device_config deviceConfig;
    ma_device device;
};

} // nes

#endif /* AUDIO_HPP */