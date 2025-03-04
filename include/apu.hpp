#ifndef APU_HPP
#define APU_HPP

#include <cstdint>
#include "nes.hpp"

namespace nes
{

struct mem_t;

extern const uint8_t length_counter_lut[];

// Cross-platform packed attribute macro
#ifdef _MSC_VER
    #define PACKED_STRUCT __pragma(pack(push, 1)) struct
    #define PACKED_STRUCT_END __pragma(pack(pop))
    #define PACKED_UNION __pragma(pack(push, 1)) union
    #define PACKED_UNION_END __pragma(pack(pop))
#else
    #define PACKED_STRUCT struct __attribute__((packed))
    #define PACKED_STRUCT_END
    #define PACKED_UNION union __attribute__((packed))
    #define PACKED_UNION_END
#endif

struct apu_t
{
    struct pulse_t {
        PACKED_UNION
        { // 0x4000  /  0x4004
            PACKED_STRUCT
            {
                uint8_t volume              : 4;
                uint8_t constant_volume     : 1;
                uint8_t length_counter_halt : 1;
                uint8_t duty                : 2;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END envelope;

        PACKED_UNION
        { // 0x4001  /  0x4005
            PACKED_STRUCT
            {
                uint8_t shift   : 3;
                uint8_t negate  : 1;
                uint8_t divider : 3;
                uint8_t enable  : 1;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END sweep;

        // 0x4002  /  0x4006
        uint8_t timer_low{0};

        PACKED_UNION
        { // 0x4003  /  0x4007
            PACKED_STRUCT
            {
                uint8_t timer_high : 3;
                uint8_t load       : 5;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END length_counter_load;

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
        PACKED_UNION
        { // 0x4008
            PACKED_STRUCT
            {
                uint8_t counter_reload : 7;
                uint8_t control        : 1;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END linear_counter_load;

        // 0x4009 unused

        // 0x400A
        uint8_t timer_low{0};

        PACKED_UNION
        { // 0x400B
            PACKED_STRUCT
            {
                uint8_t timer_high : 3;
                uint8_t load       : 5;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END length_counter_load;

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
        uint8_t  amplitude{0};

        bool muted{false};
        bool linear_counter_reload{false};
        bool length_counter_halt{false};
    };

    struct noise_t {
        PACKED_UNION
        { // 0x400C
            PACKED_STRUCT
            {
                uint8_t volume              : 4;
                uint8_t constant_volume     : 1;
                uint8_t length_counter_halt : 1;
                uint8_t unused              : 2;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END envelope;

        // 0x400D unused

        PACKED_UNION
        { // 0x400E
            PACKED_STRUCT
            {
                uint8_t period : 4;
                uint8_t unused : 3;
                uint8_t mode   : 1;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END control;

        PACKED_UNION
        { // 0x400F
            PACKED_STRUCT
            {
                uint8_t unused : 3;
                uint8_t load   : 5;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END length_counter_load;

        void write( uint16_t address, uint8_t value );
        void tick();
        void tick_length_counter();
        void tick_envelope();

        uint8_t envelope_divider{0};
        uint8_t envelope_decay{0xF};
        uint8_t length_counter{0};
        uint8_t length_counter_tmp{0};

        uint16_t period{0};
        uint16_t timer{0};
        uint16_t shift{0x0001};
        uint8_t  amplitude{0};
        uint8_t  volume{0};

        bool muted{false};
        bool start_flag{false};
        bool length_counter_halt{false};
    };

    struct dmc_t {
        PACKED_UNION
        { // 0x4010
            PACKED_STRUCT
            {
                uint8_t rate       : 4;
                uint8_t unused     : 2;
                uint8_t loop       : 1;
                uint8_t irq_enable : 1;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END control; // flags and rate

        PACKED_UNION
        { // 0x4011
            PACKED_STRUCT
            {
                uint8_t load   : 7;
                uint8_t unused : 1;
            } PACKED_STRUCT_END;
            uint8_t data{0};
        } PACKED_UNION_END direct_load;

        // 0x4012
        uint8_t sample_address{0};

        // 0x4013
        uint8_t sample_length{0};

        // Output
        uint8_t output_level{0};
        uint8_t bits_shift_register{0};
        uint8_t bits_remaining_register{0};
        bool silence{false};

        // Memory reading
        struct sample_buffer_t {
            uint8_t data{0};
            bool    empty{true};
        } sample_buffer;

        struct memory_reader_t {
            mem_t* memory{nullptr};

            uint16_t address_counter{0};
            uint16_t bytes_remaining_counter{0};

            uint8_t  tmp_data{0};
            bool data_loaded{false}; // false = get, true = store

            void tick( dmc_t* dmc );
            void start_sample( dmc_t* dmc );
        } memory_reader;

        void write( uint16_t address, uint8_t value );
        void tick();


        // Could obviously be combined to one single bool, but I like
        // the verbosity of two different bools.
        bool get_cycle{true};
        bool put_cycle{false};

        bool interrupt_flag{false};
        bool play{false};
        bool playing{false};

        uint16_t period{0};
        uint16_t timer{0};
        uint8_t amplitude{0};

    };

    PACKED_UNION
    { // 0x4015
        PACKED_STRUCT
        {
            uint8_t w_pulse_1  : 1;
            uint8_t w_pulse_2  : 1;
            uint8_t w_triangle : 1;
            uint8_t w_noise    : 1;
            uint8_t w_dmc      : 1;
            uint8_t w_unused   : 3;
        } PACKED_STRUCT_END;
        PACKED_STRUCT
        {
            uint8_t r_pulse_1  : 1;
            uint8_t r_pulse_2  : 1;
            uint8_t r_triangle : 1;
            uint8_t r_noise    : 1;
            uint8_t r_dmc      : 1;
            uint8_t r_unused   : 1;
            uint8_t r_frame_interrupt : 1;
            uint8_t r_dmc_interrupt   : 1;
        } PACKED_STRUCT_END;
        uint8_t data{0};
    } PACKED_UNION_END status;

    PACKED_UNION
    { // 0x4017
        PACKED_STRUCT
        {
            uint8_t unused            : 6;
            uint8_t interrupt_inhibit : 1;
            uint8_t sequencer_mode    : 1;
        } PACKED_STRUCT_END;
        uint8_t data{0};
    } PACKED_UNION_END frame_counter;

    void init(mem_t* mem);
    void mixer();
    void quarter_frame();
    void half_frame();
    float execute();

    pulse_t pulse_1;
    pulse_t pulse_2;
    triangle_t triangle;
    noise_t noise;
    dmc_t dmc;

    float output{0};

    mem_t* memory{nullptr};
    uint16_t cycle{0};
    uint8_t reset_frame_counter{0};
    bool frame_interrupt{false};

};

} // nes

#endif /* APU_HPP */