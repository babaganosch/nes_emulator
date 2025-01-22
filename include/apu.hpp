#ifndef APU_HPP
#define APU_HPP

#include <cstdint>
#include <miniaudio.h>

namespace nes
{

#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  44100

/*
    48000 / 60 = 800 samples per frame
    29781 / 800 = ~37.2

    44100 / 60 = 745 samples per frame
    29781 / 735 = ~40.5
*/

struct mem_t;

extern const uint8_t length_counter_lut[];

struct audio_interface_t
{
    audio_interface_t();
    ~audio_interface_t();
    
    struct audio_data_t {
        float* buffer{nullptr};
        float  amplitude{0};

        uint16_t size{0};
        uint16_t read{0};   // read  index
        uint16_t write{0};  // write index
        uint16_t length{0}; // amount of stored samples
    };

    void load( float value );

    audio_data_t data;
    ma_device_config deviceConfig;
    ma_device device;
};

struct apu_t
{
    ~apu_t();

    struct pulse_t {
        union
        { // 0x4000  /  0x4004
            struct __attribute__((packed))
            {
                uint8_t volume              : 4;
                uint8_t constant_volume     : 1;
                uint8_t length_counter_halt : 1;
                uint8_t duty                : 2;
            };
            uint8_t data{0};
        } envelope;

        union
        { // 0x4001  /  0x4005
            struct __attribute__((packed))
            {
                uint8_t shift   : 3;
                uint8_t negate  : 1;
                uint8_t divider : 3;
                uint8_t enable  : 1;
            };
            uint8_t data{0};
        } sweep;

        // 0x4002  /  0x4006
        uint8_t timer_low{0};

        union
        { // 0x4003  /  0x4007
            struct __attribute__((packed))
            {
                uint8_t timer_high : 3;
                uint8_t load       : 5;
            };
            uint8_t data{0};
        } length_counter_load;

        void write( uint16_t address, uint8_t value );
        void tick();
        void tick_length_counter();
        void tick_envelope();
        void tick_sweep( bool two_compliment );

        uint8_t envelope_divider{0};
        uint8_t envelope_decay{0xF};
        uint8_t sweep_divider{0};
        uint8_t length_counter{0};
        uint8_t length_counter_tmp{0};

        uint16_t period{0};
        uint16_t timer{0};
        uint8_t  amplitude{0};
        uint8_t  volume{0};
        uint8_t  duty{0b01111000};
        uint8_t  duty_index{0};

        bool muted{false};
        bool start_flag{false};
        bool sweep_reload{false};
        bool length_counter_halt{false};
    };

    struct triangle_t {
        union
        { // 0x4008
            struct __attribute__((packed))
            {
                uint8_t counter_reload : 7;
                uint8_t control        : 1;
            };
            uint8_t data{0};
        } linear_counter_load;

        // 0x4009 unused

        // 0x400A
        uint8_t timer_low{0};

        union
        { // 0x400B
            struct __attribute__((packed))
            {
                uint8_t timer_high : 3;
                uint8_t load       : 5;
            };
            uint8_t data{0};
        } length_counter_load;

        void write( uint16_t address, uint8_t value );
        void tick();
        void tick_linear_counter();
        void tick_length_counter();

        uint8_t linear_counter{0};
        uint8_t length_counter{0};
        uint8_t length_counter_tmp{0};

        uint16_t period{0};
        uint16_t timer{0};
        uint8_t  period_index{0};

        bool muted{false};
        bool linear_counter_reload{false};
        bool length_counter_halt{false};
    };

    union
    { // 0x4015
        struct __attribute__((packed))
        {
            uint8_t w_pulse_1  : 1;
            uint8_t w_pulse_2  : 1;
            uint8_t w_triangle : 1;
            uint8_t w_noise    : 1;
            uint8_t w_dmc      : 1;
            uint8_t w_unused   : 3;
        }; // For write-only
        struct __attribute__((packed))
        {
            uint8_t r_pulse_1  : 1;
            uint8_t r_pulse_2  : 1;
            uint8_t r_triangle : 1;
            uint8_t r_noise    : 1;
            uint8_t r_dmc      : 1;
            uint8_t r_unused   : 1;
            uint8_t r_frame_interrupt : 1;
            uint8_t r_dmc_interrupt   : 1;
        }; // For read-only
        uint8_t data{0};
    } status;

    union
    { // 0x4017
        struct __attribute__((packed))
        {
            uint8_t unused            : 6;
            uint8_t interrupt_inhibit : 1;
            uint8_t sequencer_mode    : 1;
        };
        uint8_t data{0};
    } frame_counter;

    void init(mem_t &mem);
    void mixer();
    void execute();
    void quarter_frame();
    void half_frame();

    pulse_t pulse_1;
    pulse_t pulse_2;
    triangle_t triangle;
    // noise
    // dmc

    audio_interface_t* audio{nullptr};

    mem_t* memory{nullptr};
    uint16_t cycle{0};
    uint8_t reset_frame_counter{0};
    bool frame_interrupt{false};
    float audio_sample_timer{0};

};

} // nes

#endif /* APU_HPP */