#include "nes.hpp"
#include "nes_ops.hpp"

#include <cstdio>

namespace nes
{

void cpu_t::init(callback_t cb)
{
    callback = cb;
    printf("cpu.init()\n");
}

void cpu_t::execute()
{
    printf("cpu.execute()\n");
    
    uint16_t operand = op_codes[0].addr_mode( 0xAA, 0xAB );
    op_codes[0].function(*this, memory, operand );

    tick_clock();
    tick_clock();
    tick_clock();
}

} // nes