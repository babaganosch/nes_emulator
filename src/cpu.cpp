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

    regs.PC = 0xC000;
    regs.SP = 0xFD;
    regs.SR = 0x24;
    regs.A = regs.X = regs.Y = 0x0;
    cycles = 0;
}

void cpu_t::execute()
{
    printf("cpu.execute()\n");
    printf("PRE INSTR CYCLES: %u - A: %u\n", cycles, regs.A);
    // Fetch instruction
    uint8_t instruction = fetch_byte( regs.PC++ );

    uint16_t operand = op_codes[instruction].addr_mode(*this);
    op_codes[instruction].function(*this, operand );

    printf("POST INSTR CYCLES: %u - A: %u\n", cycles, regs.A);
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
    uint8_t data = (*memory)[ address ];
    printf("FETCH BYTE @ %04x : %u\n", address, data);
    tick_clock();
    return data;
}

uint8_t cpu_t::fetch_byte( uint8_t lo, uint8_t hi )
{
    uint16_t address = (hi << 8) | lo;
    uint8_t data = (*memory)[ address ];
    printf("FETCH BYTE @ %04x : %u\n", address, data);
    tick_clock();
    return data;
}

} // nes