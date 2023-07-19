#ifndef NES_H
#define NES_H

#include <cstdint>
#include <memory>

namespace nes
{

enum RESULT
{
    RESULT_INVALID_INES_HEADER = -1,
    RESULT_ERROR = 0,

    RESULT_OK    = 1
};

typedef void (* callback_t)(void * cookie);

struct mem_t
{
    /*
    NES CPU address space
    https://wiki.nesdev.com/w/index.php/CPU_memory_map

    +---------------+ (0x10000)
    |    PRG ROM    |
    |  (upper bank) |          char* prgrom_upper;
    +---------------+ 0xC000
    |    PRG ROM    |
    |  (lower bank) |          char* prgrom_lower;
    +---------------+ 0x8000
    |      SRAM     |
    +---------------+ 0x6000
    | Expansion ROM |
    +---------------+ 0x4020
    | I/O Registers |
    +---------------+ 0x4000
    |    Mirrors    |
    | 0x2000-0x2007 |
    +---------------+ 0x0800
    |      RAM      |
    +---------------+ 0x0200
    |     Stack     |
    +---------------+ 0x0100
    |   Zero Page   |
    +---------------+ 0x0000

    */
    union
    {
        struct
        {
            union
            {
                struct
                {
                    uint8_t zero_page         [0x0100]; // $0000 - $00FF
                    uint8_t stack             [0x0100]; // $0100 - $01FF
                    uint8_t ram               [0x0600]; // $0200 - $07FF
                };
                uint8_t internal_ram          [0x0800]; // $0000 - $07FF
            };
            
            uint8_t ram_mirrors               [0x1800]; // $0800 - $1FFF (repeats every $800 bytes)

            uint8_t ppu_regs                  [0x0008]; // $2000 - $2007
            uint8_t ppu_mirrors               [0x1FF8]; // $2008 – $3FFF (repeats every 8 bytes)

            union
            {
                struct
                {
                    uint8_t apu_pulse1        [0x0004]; // $4000 – $4003
                    uint8_t apu_pulse2        [0x0004]; // $4004 – $4007
                    uint8_t apu_triangle      [0x0004]; // $4008 – $400B
                    uint8_t apu_noise         [0x0004]; // $400C – $400F
                    uint8_t apu_dmc           [0x0004]; // $4010 – $4013
                    uint8_t apu_status        [0x0002]; // $4015 - $4016?
                    uint8_t apu_frame_counter [0x0001]; // $4017
                };
                uint8_t apu_regs              [0x0018]; // $4000 – $4017
            };
            uint8_t cpu_test_mode             [0x0008]; // $4018 – $401F

            union
            {
                struct
                {
                    uint8_t expansion_rom    [0x1FE0];  // $4020 - $5FFF
                    uint8_t sram             [0x2000];  // $6000 - $7FFF
                    uint8_t prg_lower_bank   [0x4000];  // $8000 - $BFFF
                    uint8_t prg_upper_bank   [0x4000];  // $C000 - $FFFF
                };
                uint8_t cartridge_space      [0xBFE0];  // $4020 – $FFFF
            };
        };
        uint8_t data[0x10000];
    };

    uint8_t  operator[] (uint32_t address) const { return data[address]; };
    uint8_t &operator[] (uint32_t address)       { return data[address]; };

    void init();
};

struct cpu_t
{
    // Registers
    struct regs_t
    {
        uint8_t A; // Accumulator
        uint8_t X; // X
        uint8_t Y; // Y

        uint8_t  SP; // Stack Pointer
        uint16_t PC; // Program Counter

        // Status Register
        //  7 6 5 4 3 2 1 0
        //  N V - - D I Z C
        union 
        {
            struct 
            {
                uint8_t C : 1; // Carry 
                uint8_t Z : 1; // Zero
                uint8_t I : 1; // Interrupt Disable
                uint8_t D : 1; // Decimal
                uint8_t B : 2; // -- (Break)
                uint8_t V : 1; // Overflow
                uint8_t N : 1; // Negative
            };
            uint8_t SR;
        };
    } regs;

    struct vectors_t
    {
        uint16_t NMI;
        uint16_t RESET;
        uint16_t IRQBRK;
    } vectors;

    uint16_t cycles{0};
    callback_t callback{nullptr};

    mem_t* memory{nullptr};
    
    uint8_t fetch_byte( uint16_t address );
    void tick_clock();
    void init(callback_t cb, mem_t &mem);
    void execute();
};



struct ines_rom_t
{
    struct header_t
    { // 16 bytes
        uint8_t magic[4];   // ASCII "NES" followed by MS-DOS end-of-file
        uint8_t prg_size;   // Size of PRG ROM in 16 KB units (16384 * x bytes)
        uint8_t chr_size;   // Size of CHR ROM in 8 KB units (8192 * y bytes) (value 0 means the board uses CHR RAM)
        uint8_t flags_6;    // Mapper, mirroring, battery, trainer
        uint8_t flags_7;    // Mapper, VS/Playchoice, NES 2.0
        uint8_t flags_8;    // PRG-RAM size (rarely used extension)
        uint8_t flags_9;    // TV system (rarely used extension)
        uint8_t flags_10;   // TV system, PRG-RAM presence (unofficial, rarely used extension)
        uint8_t padding[5]; // Unused padding
    } header;

    // Data is stored in pages, each page 16KB (16 * 1024 * x bytes)
    uint8_t** prg_pages{nullptr};
    uint8_t** chr_pages{nullptr};

    ~ines_rom_t();

    void clear_contents();
    RESULT load_from_file(const char* filepath);
    RESULT load_from_data(const uint8_t* data, const uint32_t size);
};

struct emu_t
{
    cpu_t cpu;
    mem_t memory;

    RESULT init(ines_rom_t &rom);
    RESULT step(uint16_t cycles);
};

} // nes

#endif /* NES_H */