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

    // Fetch instruction
    uint8_t ins_num = fetch_byte( regs.PC++ );

    // Perform instruction
    op_code_t& op_code = op_codes[ins_num];
    op_code.function(*this, op_code.addr_mode );

    printf("X: %02x Y: %02x A: %02x\n", regs.X, regs.Y, regs.A);
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
    printf("FETCH BYTE @ %04x : %02x\n", address, data);
    tick_clock();
    return data;
}

uint8_t cpu_t::fetch_byte( uint8_t lo, uint8_t hi )
{
    uint16_t address = (hi << 8) | lo;
    uint8_t data = (*memory)[ address ];
    printf("FETCH BYTE @ %04x : %02x\n", address, data);
    tick_clock();
    return data;
}

uint8_t* cpu_t::fetch_byte_ref( uint16_t address )
{
    tick_clock();
    return &(*memory)[ address ];
}

uint8_t* cpu_t::fetch_byte_ref( uint8_t lo, uint8_t hi )
{
    tick_clock();
    uint16_t address = (hi << 8) | lo;
    return &(*memory)[ address ];
}

void cpu_t::write_byte( uint8_t data, uint8_t* ref )
{
    *ref = data;
    tick_clock();
}

void cpu_t::write_byte( uint8_t data, uint16_t address )
{
    (*memory)[ address ] = data;
    printf("WRITE BYTE @ %04x : %02x\n", address, data);
    tick_clock();
}

void cpu_t::write_byte( uint8_t data, uint8_t lo, uint8_t hi )
{
    uint16_t address = (hi << 8) | lo;
    (*memory)[ address ] = data;
    printf("WRITE BYTE @ %04x : %02x\n", address, data);
    tick_clock();
}

} // nes