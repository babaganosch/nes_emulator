#include "nes.hpp"
#include "nes_ops.hpp"

#include <cstdio>

namespace nes
{

void cpu_t::init(cpu_callback_t cb, mem_t &mem)
{
    callback = cb;
    memory = &mem;

    regs.PC = 0xC000;
    regs.SP = 0xFD;
    regs.SR = 0x24;
    regs.A = regs.X = regs.Y = 0x0;
    cycles = 0;
}

void cpu_t::execute()
{
    uint16_t start_cycles = cycles;

    // Fetch instruction
    uint8_t ins_num = fetch_byte( regs.PC++ );

    // Perform instruction
    op_code_t& op_code = op_codes[ins_num];
    op_code.function(*this, op_code.addr_mode );

    // Minimum cycles is 2 per instruction
    while (cycles - start_cycles < 2)
    {
        tick_clock();
    }
}

void cpu_t::tick_clock()
{
    cycles++; 
    if (callback) 
    { // PPU should run at tripple the CPU clock speed
        callback(nullptr);
        callback(nullptr);
        callback(nullptr);
    }
}

void cpu_t::tick_clock( uint8_t cycles )
{
    while ( cycles-- > 0 )
    {
        tick_clock();
    }
}

uint16_t cpu_t::peek_memory( uint16_t address )
{
    return memory->memory_read( address );
}

uint8_t cpu_t::fetch_byte( uint16_t address )
{
    uint8_t data = memory->memory_read( address );
    tick_clock();
    return data;
}

uint8_t cpu_t::fetch_byte( uint8_t lo, uint8_t hi )
{
    uint16_t address = ((uint16_t)hi << 8) | lo;
    uint8_t data = memory->memory_read( address );
    tick_clock();
    return data;
}

uint8_t* cpu_t::fetch_byte_ref( uint16_t address )
{
    tick_clock();
    return &(*memory)[ address ];
}

void cpu_t::write_byte( uint8_t data, uint8_t* ref )
{
    *ref = data;
    tick_clock();
}

void cpu_t::write_byte( uint8_t data, uint16_t address )
{
    memory->memory_write( data, address );
    tick_clock();
}

void cpu_t::write_byte( uint8_t data, uint8_t lo, uint8_t hi )
{
    uint16_t address = ((uint16_t)hi << 8) | lo;
    memory->memory_write( data, address );
    tick_clock();
}

void cpu_t::push_byte_to_stack( uint8_t data )
{
    uint8_t address = regs.SP--;
    tick_clock();
    (*memory).stack[ address ] = data;
}

void cpu_t::push_short_to_stack( uint16_t data )
{
    push_byte_to_stack( (0xFF00 & data) >> 8 );
    push_byte_to_stack( 0x00FF & data );
}

uint8_t cpu_t::pull_byte_from_stack()
{
    uint8_t address = ++regs.SP;
    tick_clock();
    uint8_t data = (*memory).stack[ address ];
    return data;
}

uint16_t cpu_t::pull_short_from_stack()
{
    uint8_t lo = pull_byte_from_stack();
    uint8_t hi = pull_byte_from_stack();
    return ((uint16_t)hi << 8) | lo;
}

} // nes