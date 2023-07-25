#include "nes.hpp"
#include "nes_ops.hpp"

#include <cstdio>

namespace nes
{

void cpu_t::init(cpu_callback_t cb, mem_t &mem)
{
    callback = cb;
    memory = &mem;

    regs.SR = 0x34;
    regs.A  = regs.X = regs.Y = 0x0;
    regs.SP = 0xFD;
    regs.PC = 0xFFFC;

    cycles = 0;
}

void cpu_t::execute()
{
    // Fetch instruction
    uint8_t ins_num = fetch_byte( regs.PC++ );

    // Perform instruction
    op_code_t& op_code = op_codes[ins_num];
    op_code.function(*this, op_code.addr_mode );
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

void cpu_t::tick_clock( uint8_t ticks )
{
    while ( ticks-- > 0 )
    {
        tick_clock();
    }
}

uint8_t cpu_t::peek_byte( uint16_t address )
{
    uint8_t* ptr = memory->memory_read( mem_t::CPU, address );
    return ptr == nullptr ? 0xFF : *ptr;
}

uint16_t cpu_t::peek_short( uint16_t address )
{
    uint8_t* ptr = memory->memory_read( mem_t::CPU, address );
    if (ptr == nullptr) return 0xFFFF;
    uint16_t ret = *ptr;
    ptr = memory->memory_read( mem_t::CPU, address + 1 );
    if (ptr == nullptr) return 0xFFFF;
    return *ptr << 8 | ret;
}

uint8_t cpu_t::fetch_byte( uint16_t address )
{
    uint8_t* ptr = memory->memory_read( mem_t::CPU, address );
    uint8_t data = ptr == nullptr ? 0xFF : *ptr;
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
    return memory->memory_read( mem_t::CPU, address );
}

void cpu_t::write_byte( uint8_t data, uint8_t* ref )
{
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
        printf("--- ERROR: Stack Overflow! ---\n");
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