#include "nes.hpp"
#include "nes_ops.hpp"
#include "logging.hpp"

#include <cstdio>

namespace nes
{

void cpu_t::init(cpu_callback_t cb, mem_t &mem)
{
    callback = cb;
    memory = &mem;
    memory->cpu = this;

    // Try to grab the interrupt vectors
    vectors.NMI = peek_short( 0xFFFA );
    vectors.RESET = peek_short( 0xFFFC );
    vectors.IRQBRK = peek_short( 0xFFFE );

    regs.SR = 0x24;
    regs.A  = regs.X = regs.Y = 0x00;
    regs.SP = 0xFD;
    regs.PC = vectors.RESET;

    cycles = 0u;

    nmi_control.pending = false;
    nmi_control.trigger_countdown = 0u;
}

uint16_t cpu_t::execute()
{
    // Reset CPU instruction delta
    delta_cycles = 0u;

    // Fetch instruction
    uint8_t ins_num = fetch_byte( regs.PC++ );
    
    // Perform instruction
    op_code_t& op_code = op_codes[ins_num];
    op_code.function(*this, op_code.addr_mode );

    // Has NMI occurred?
    if ( nmi_control.pending && !nmi_control.trigger_countdown ) {
        nmi_control.pending = false;
        nmi();
    }

    return delta_cycles;
}

void cpu_t::tick_clock()
{
    delta_cycles++;
    cycles++;
    memory->cpu_cycles = cycles;
    if (callback)
    { // NTSC PPU runs at 3x the CPU clock speed
        callback(nullptr);
        callback(nullptr);
        callback(nullptr);
        if (variant == PAL)
        { // PAL PPU runs at 3.2x the CPU clock speed
            pal_clock_buffer++;
            if (pal_clock_buffer >= 5)
            {
                callback(nullptr);
                pal_clock_buffer = 0;
            }
        }
    }
}

void cpu_t::tick_clock( uint8_t ticks )
{
    while ( ticks-- > 0 )
    {
        tick_clock();
    }
}

void cpu_t::nmi()
{ // non-maskable interrupt
    push_short_to_stack( regs.PC );
    push_byte_to_stack( regs.SR );
    regs.I  = 1;
    regs.PC = vectors.NMI;
}

uint8_t cpu_t::peek_byte( uint16_t address )
{
    return memory->memory_read( mem_t::CPU, address, true );
}

uint16_t cpu_t::peek_short( uint16_t address )
{
    uint16_t lo = memory->memory_read( mem_t::CPU, address,     true );
    uint16_t hi = memory->memory_read( mem_t::CPU, address + 1, true ) << 8;
    return hi | lo;
}

uint8_t cpu_t::fetch_byte( uint16_t address )
{
    uint8_t data = memory->memory_read( mem_t::CPU, address, false );
    tick_clock();
    return data;
}

uint8_t cpu_t::fetch_byte( uint8_t lo, uint8_t hi )
{
    uint16_t address = ((uint16_t)hi << 8) | lo;
    return fetch_byte( address );
}

uint8_t* cpu_t::fetch_byte_ref( uint16_t address )
{
    tick_clock();
    if ( address < 0x2000 )
    { // internal ram
        return &memory->cpu_mem.internal_ram[ address ];
    }
    else if ( address < 0x4020 )
    {
        LOG_E("Trying to fetch unmapped reference (%04x)", address);
    }
    else if ( address < 0x6000 )
    { // expansion rom
        return &memory->cartridge_mem.expansion_rom[ address - 0x4020 ];
    }
    else if ( address < 0x8000 )
    { // sram
        return &memory->cartridge_mem.sram[ address - 0x6000 ];
    }
    else if ( address < 0xC000 )
    { // prg lower
        return &memory->cartridge_mem.prg_lower_bank[ address - 0x8000 ];
    }
    else if ( address <= 0xFFFF )
    { // prg upper
        return &memory->cartridge_mem.prg_upper_bank[ address - 0xC000 ];
    }
    return nullptr;
}

void cpu_t::write_byte( uint8_t data, uint8_t* ref )
{
    if (!ref)
    {
        LOG_E("CPU write byte to nullptr");
        throw RESULT_ERROR;
    }
    
    *ref = data;
    tick_clock();
}

void cpu_t::write_byte( uint8_t data, uint16_t address )
{
    memory->memory_write( mem_t::CPU, data, address );
    tick_clock();
}

void cpu_t::write_byte( uint8_t data, uint8_t lo, uint8_t hi )
{
    uint16_t address = ((uint16_t)hi << 8) | lo;
    memory->memory_write( mem_t::CPU, data, address );
    tick_clock();
}

void cpu_t::push_byte_to_stack( uint8_t data )
{
    if ( regs.SP == 0x00 )
    {
        LOG_E("--- Stack Overflow! ---");
        throw;
    }
    uint8_t address = regs.SP--;
    tick_clock();
    (*memory).cpu_mem.stack[ address ] = data;
}

void cpu_t::push_short_to_stack( uint16_t data )
{
    push_byte_to_stack( (0xFF00 & data) >> 8 );
    push_byte_to_stack( 0x00FF & data );
}

uint8_t cpu_t::pull_byte_from_stack()
{
    if ( regs.SP == 0xFF )
    {
        LOG_E("--- Stack Underflow! ---");
        throw;
    }
    uint8_t address = ++regs.SP;
    tick_clock();
    uint8_t data = (*memory).cpu_mem.stack[ address ];
    return data;
}

uint16_t cpu_t::pull_short_from_stack()
{
    uint8_t lo = pull_byte_from_stack();
    uint8_t hi = pull_byte_from_stack();
    return ((uint16_t)hi << 8) | lo;
}

} // nes