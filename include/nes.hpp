#ifndef NES_H
#define NES_H

#include <cstdint>
#include <memory>

namespace nes
{

enum RESULT
{
    RESULT_VALIDATION_SUCCESS  = -20,
    RESULT_INVALID_INES_HEADER = -10,
    RESULT_INVALID_ARGUMENTS   = -1,
    RESULT_ERROR               = 0,

    RESULT_OK                  = 1,
};

typedef void (* cpu_callback_t)(void * cookie);

struct mem_t
{
    /*
    NES CPU address space
    https://wiki.nesdev.com/w/index.php/CPU_memory_map

    +---------------+ (0x10000)
    |    PRG ROM    |
    |  (upper bank) | 
    +---------------+ 0xC000
    |    PRG ROM    |
    |  (lower bank) | 
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

    struct cpu_mem_t
    { // $0000 - $401F
        union
        {
            struct
            {
                uint8_t zero_page         [0x0100]; // $0000 - $00FF
                uint8_t stack             [0x0100]; // $0100 - $01FF
                uint8_t ram               [0x0600]; // $0200 - $07FF
            };
            uint8_t internal_ram          [0x0800]; // $0000 - $07FF
            // Mirroring $0800 - $1FFF of $0000 - $07FF
            // (repeats every $800 bytes)
        };

        // $2000 - $2007
        struct
        {
            uint8_t* PPUCTRL;
            uint8_t* PPUMASK;
            uint8_t* PPUSTATUS;
            uint8_t* OAMADDR;
            uint8_t* OAMDATA;
            uint8_t* PPUSCROLL;
            uint8_t* PPUADDR;
            uint8_t* PPUDATA;
            uint8_t* OAMDMA;
        } ppu_regs;
        // Mirroring $2008 – $3FFF of $2000 - $2007 
        // (repeats every 8 bytes)

        // $4000 – $4017
        uint8_t* apu_regs{nullptr};

        // $4018 – $401F
        uint8_t* cpu_test_mode{nullptr}; 

    } cpu_mem;

    struct ppu_mem_t
    {
        uint8_t vram[0x800];
        uint8_t oam[64 * 4];
    } ppu_mem;

    struct apu_mem_t
    {
             
    } apu_mem;

    struct cartridge_mem_t
    {
        // PPU: $0000 - $1FFF
        uint8_t* chr_rom;          // PPU: $0000 - $1FFF

        // CPU: $4020 - $FFFF
        uint8_t* expansion_rom;    // CPU: $4020 - $5FFF
        uint8_t* sram;             // CPU: $6000 - $7FFF
        uint8_t* prg_lower_bank;   // CPU: $8000 - $BFFF
        uint8_t* prg_upper_bank;   // CPU: $C000 - $FFFF
    } cartridge_mem;

    enum MEMORY_BUS
    {
        CPU,
        PPU,
        APU
    };

    void     init();

    uint8_t* memory_read( MEMORY_BUS bus, uint16_t address );
    void     memory_write( MEMORY_BUS bus, uint8_t data, uint16_t address );

    uint8_t* cpu_memory_read( uint16_t address );
    void     cpu_memory_write( uint8_t data, uint16_t address );

    uint8_t* ppu_memory_read( uint16_t address );
    void     ppu_memory_write( uint8_t data, uint16_t address );
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
    cpu_callback_t callback{nullptr};

    mem_t* memory{nullptr};
    
    void tick_clock();
    void tick_clock( uint8_t cycles );
    void init(cpu_callback_t cb, mem_t &mem);
    void execute();

    uint8_t  peek_byte( uint16_t address );
    uint16_t peek_short( uint16_t address );
    uint8_t  fetch_byte( uint16_t address );
    uint8_t  fetch_byte( uint8_t lo, uint8_t hi );
    uint8_t* fetch_byte_ref( uint16_t address );
    uint8_t  pull_byte_from_stack();
    uint16_t pull_short_from_stack();
    void     write_byte( uint8_t data, uint8_t* ref );
    void     write_byte( uint8_t data, uint16_t address );
    void     write_byte( uint8_t data, uint8_t lo, uint8_t hi );
    void     push_byte_to_stack( uint8_t data );
    void     push_short_to_stack( uint16_t data );

    // Validation Helpers
    bool nestest_validation{false};
    char nestest_validation_str[28];
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
    void load_from_file(const char* filepath);
    void load_from_data(const uint8_t* data, const uint32_t size);
};

struct apu_t
{

};

struct ppu_t
{
    uint16_t x{0};
    uint16_t y{0};
    uint32_t cycles{0};

    mem_t* memory{nullptr};

    struct regs_t
    {
        uint8_t PPUCTRL;
        uint8_t PPUMASK;
        uint8_t PPUSTATUS;
        uint8_t OAMADDR;
        uint8_t OAMDATA;
        uint8_t PPUSCROLL;
        uint8_t PPUADDR;
        uint8_t PPUDATA;
        uint8_t OAMDMA;
    } regs;

    RESULT init(mem_t &mem);
    RESULT execute();
};

struct emu_t
{
    cpu_t cpu;
    ppu_t ppu;
    mem_t memory;

    RESULT init(ines_rom_t &rom);
    RESULT step(uint16_t cycles);
};

} // nes

#endif /* NES_H */