#include "nes.hpp"
#include "nes_ops.hpp"

#include <cstdio>

namespace nes
{

void cpu_t::init(callback_t cb, mem_t &mem)
{
    printf("cpu.init()\n");
    callback = cb;
    memory = &mem;

    regs.PC = 0xFFFF;
    regs.SP = 0xFD;
    regs.SR = 0x24;
    regs.A = regs.X = regs.Y = 0x0;
    cycles = 0;
}

void cpu_t::execute()
{
    printf("cpu.execute()\n");

    // Fetch instruction
    uint8_t instruction = fetch_byte( regs.PC++ );

    uint16_t operand = op_codes[instruction].addr_mode( 0xAA, 0xAB );
    op_codes[instruction].function(*this, memory, operand );
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

uint8_t cpu_t::fetch_byte( uint16_t address )
{
    mem_t& mem = *memory;
    uint8_t data = mem[ address ];
    printf("DATA: %04x : %u\n", address, data);
    tick_clock();
    return data;
}

} // nes