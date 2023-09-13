#ifndef NES_HPP
#define NES_HPP

#include <cstdint>
#include <memory>

namespace nes
{

#define NES_WIDTH  256
#define NES_HEIGHT 240

#define BIT_CHECK_HI(value, bit) (((value >> bit) & 0x1) == 0x1)
#define BIT_CHECK_LO(value, bit) (((value >> bit) & 0x1) == 0x0)
#define UINT16(LO, HI) (((uint16_t) HI << 8) | LO)

extern uint32_t window_buffer[];

enum RESULT
{
    RESULT_INVALID_INES_HEADER = -10,

    RESULT_MFB_ERROR           = -3,
    RESULT_INVALID_ARGUMENTS   = -2,
    RESULT_ERROR               = -1,

    RESULT_OK                  = 0,
    RESULT_VALIDATION_SUCCESS  = 1
};

inline const char* RESULT_to_string(RESULT v)
{
    switch (v)
    {
        case RESULT_INVALID_INES_HEADER:    return "[Invalid iNES Header]";
        case RESULT_MFB_ERROR:              return "[MFB Error]";
        case RESULT_INVALID_ARGUMENTS:      return "[Invalid Arguments]";
        case RESULT_ERROR:                  return "[Error]";
        case RESULT_OK:                     return "[OK]";
        case RESULT_VALIDATION_SUCCESS:     return "[Validation Success]";
        default:                            return "[Unkown]";
    }
}

typedef void (* cpu_callback_t)(void * cookie);
struct cpu_t;
struct ppu_t;
struct apu_t;
struct ines_rom_t;

struct gamepad_t
{
    union 
    {
        struct __attribute__((packed))
        {
            uint8_t A : 1;
            uint8_t B : 1;
            uint8_t select : 1;
            uint8_t start : 1;
            uint8_t up : 1;
            uint8_t down : 1;
            uint8_t left : 1;
            uint8_t right : 1;
        };
        uint8_t data{0};
    };

    uint8_t latch{0};
};

struct cpu_mem_t
{ // $0000 - $401F
    union
    {
        struct __attribute__((packed))
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
    // PPU Registers
    // Mirroring $2008 – $3FFF of $2000 - $2007 
    // (repeats every 8 bytes)

    // $4000 – $4017
    uint8_t* apu_regs{nullptr};

    // $4018 – $401F
    uint8_t* cpu_test_mode{nullptr}; 
};

union oam_t
{ // Primary OAM, holds 64 sprites, each sprite 4 bytes
    uint8_t data [64*4];
    uint8_t arr2d[64][4];
};

union soam_t
{ // Secondary OAM, holds 8 sprites for current scanline
    uint8_t data [8*4];
    uint8_t arr2d[8][4];
};

struct ppu_mem_t
{
    uint8_t palette [0xFF];
    uint8_t vram    [0x800];
    oam_t   oam;
    soam_t  soam;
    

    /* PPU Shift register VRAM address and Temp VRAM address
     *   yyy NN YYYYY XXXXX
     *   ||| || ||||| +++++-- coarse X scroll
     *   ||| || +++++-------- coarse Y scroll
     *   ||| ++-------------- nametable select
     *   +++----------------- fine Y scroll
     */
    union vram_shift_regs_t
    {
        struct __attribute__((packed))
        {
            uint16_t coarse_x  : 5;
            uint16_t coarse_y  : 5;
            uint16_t nametable : 2;
            uint16_t fine_y    : 3;
            uint16_t padding   : 1;
        };
        uint16_t data{0};
    };

    enum class nametable_mirroring
    {
        horizontal    = 0,
        vertical      = 1,
        single_screen = 2,
        four_screen   = 3
    };

    vram_shift_regs_t v; // Current vram address
    vram_shift_regs_t t; // Temporary vram address
    uint8_t fine_x{0};   // Fine X scroll
    uint8_t w{0};        // Write toggle

    uint8_t write_latch{0};
    uint8_t ppudata_read_buffer{0};
    nametable_mirroring nt_mirroring{nametable_mirroring::horizontal};
};

struct cartridge_mem_t
{
    // PPU: $0000 - $1FFF
    uint8_t* chr_rom;                 // PPU: $0000 - $1FFF

    // CPU: $4020 - $FFFF
    uint8_t expansion_rom[0x1FE0];    // CPU: $4020 - $5FFF
    uint8_t sram[0x2000];             // CPU: $6000 - $7FFF
    uint8_t* prg_lower_bank;          // CPU: $8000 - $BFFF
    uint8_t* prg_upper_bank;          // CPU: $C000 - $FFFF
};

struct apu_mem_t
{
    // Not implemented
};

struct mem_t
{
    cpu_t* cpu;
    cpu_mem_t cpu_mem;

    ppu_t* ppu;
    ppu_mem_t ppu_mem;

    apu_t* apu;
    apu_mem_t apu_mem;

    ines_rom_t* ines_rom;
    cartridge_mem_t cartridge_mem;

    gamepad_t gamepad[2];
    uint8_t   gamepad_strobe{0};
    uint32_t  cpu_cycles{0};

    enum MEMORY_BUS
    {
        CPU,
        PPU,
        APU
    };

    void     init( ines_rom_t &rom );

    uint8_t  memory_read( MEMORY_BUS bus, uint16_t address, bool peek );
    void     memory_write( MEMORY_BUS bus, uint8_t data, uint16_t address );

    uint8_t  cpu_memory_read( uint16_t address, bool peek );
    void     cpu_memory_write( uint8_t data, uint16_t address );

    uint8_t  ppu_memory_read( uint16_t address, bool peek );
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
            struct __attribute__((packed))
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

    enum VARIANT
    {
        NTSC,
        PAL
    };

    uint32_t cycles{0};
    uint16_t delta_cycles{0};
    cpu_callback_t callback{nullptr};
    bool queue_nmi{false};
    VARIANT variant{NTSC};
    uint16_t pal_clock_buffer{0};

    mem_t* memory{nullptr};
    
    void tick_clock();
    void tick_clock( uint8_t cycles );
    void init(cpu_callback_t cb, mem_t &mem);
    void nmi();
    uint16_t execute();

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

struct ppu_t
{

    struct regs_t
    {
        /* PPUCTRL > write
        *  7  bit  0
        *  ---- ----
        *  VPHB SINN
        *  |||| ||||
        *  |||| ||++- Base nametable address
        *  |||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
        *  |||| |+--- VRAM address increment per CPU read/write of PPUDATA
        *  |||| |     (0: add 1, going across; 1: add 32, going down)
        *  |||| +---- Sprite pattern table address for 8x8 sprites
        *  ||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
        *  |||+------ Background pattern table address (0: $0000; 1: $1000)
        *  ||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM#Byte 1)
        *  |+-------- PPU master/slave select
        *  |          (0: read backdrop from EXT pins; 1: output color on EXT pins)
        *  +--------- Generate an NMI at the start of the
        *             vertical blanking interval (0: off; 1: on)
        */
        uint8_t PPUCTRL;
        /* PPUMASK > write
        * 7  bit  0
        * ---- ----
        * BGRs bMmG
        * |||| ||||
        * |||| |||+- Greyscale (0: normal color, 1: produce a greyscale display)
        * |||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
        * |||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
        * |||| +---- 1: Show background
        * |||+------ 1: Show sprites
        * ||+------- Emphasize red (green on PAL/Dendy)
        * |+-------- Emphasize green (red on PAL/Dendy)
        * +--------- Emphasize blue
        */
        uint8_t PPUMASK;
        /* PPUSTATUS < read
        * 7  bit  0
        * ---- ----
        * VSO. ....
        * |||| ||||
        * |||+-++++- PPU open bus. Returns stale PPU bus contents.
        * ||+------- Sprite overflow. The intent was for this flag to be set
        * ||         whenever more than eight sprites appear on a scanline, but a
        * ||         hardware bug causes the actual behavior to be more complicated
        * ||         and generate false positives as well as false negatives; see
        * ||         PPU sprite evaluation. This flag is set during sprite
        * ||         evaluation and cleared at dot 1 (the second dot) of the
        * ||         pre-render line.
        * |+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
        * |          a nonzero background pixel; cleared at dot 1 of the pre-render
        * |          line.  Used for raster timing.
        * +--------- Vertical blank has started (0: not in vblank; 1: in vblank).
        *         Set at dot 1 of line 241 (the line *after* the post-render
        *         line); cleared after reading $2002 and at dot 1 of the
        *         pre-render line.
        */
        uint8_t PPUSTATUS;
        uint8_t OAMADDR;
        uint8_t OAMDATA;
        uint8_t OAMDMA;

    } regs;

    struct shift_regs_t
    {
        union pt_t
        {
            struct __attribute__((packed))
            {
                uint16_t lo : 8;
                uint16_t hi : 8;
            };
            uint16_t data;
        };
        // BGs
        pt_t pt_hi;
        pt_t pt_lo;
        uint8_t  at_hi;
        uint8_t  at_lo;
        // Sprites
        uint8_t sprite_pattern_tables_lo[8];
        uint8_t sprite_pattern_tables_hi[8];
    } shift_regs;

    struct latch_t
    {
        // BGs
        uint8_t  nt_latch;
        uint8_t  at_latch;
        uint8_t  at_byte;
        uint16_t pt_latch;
        // Sprites
        uint8_t  sprite_attribute_latch[8];
    } latches;

    enum class render_states
    {
        pre_render_scanline    = 0, // (-1 or 261)
        visible_scanline       = 1, // (0 - 239)
        post_render_scanline   = 2, // (240)
        vertical_blanking_line = 3  // (241 - 260)
    };

    uint16_t x{0};
    uint16_t y{0};
    uint16_t vram_address_multiplexer{0};
    uint32_t cycles{0};

    uint8_t  oam_n{0};
    uint8_t  oam_m{0};
    uint8_t  oam_read_buffer[4]{0};
    uint8_t  soam_counter{0};
    uint8_t  sprite_fetch{0};
    int16_t  sprite_counters[8];

    mem_t* memory{nullptr};
    bool recently_power_on{false};
    uint8_t frame_num{0};
    render_states render_state{render_states::pre_render_scanline};

    uint32_t bg_color{0};
    uint32_t sp_color{0};
    bool sprite0{false};
    void bg_evaluation( uint16_t dot, uint16_t scanline );
    void sp_evaluation( uint16_t dot, uint16_t scanline );
    void render( uint16_t dot, u_int16_t scanline );

    // Rendering cycle stages from PPU Frame Timing Diagram
    // https://www.nesdev.org/wiki/PPU_rendering
    void vram_fetch_nt( bool step );
    void vram_fetch_at( bool step );
    void vram_fetch_bg_lsbits( bool step );
    void vram_fetch_bg_msbits( bool step );
    void v_update_inc_hori_v();
    void v_update_inc_vert_v();
    void v_update_hori_v_eq_hori_t();
    void v_update_vert_v_eq_vert_t();
    void reload_shift_registers();
    
    RESULT init(mem_t &mem);
    RESULT execute();
};

struct emu_t
{
    cpu_t cpu;
    ppu_t ppu;
    mem_t memory;

    void init(ines_rom_t &rom);
    RESULT step(int32_t cycles);
};

} // nes

#endif /* NES_HPP */