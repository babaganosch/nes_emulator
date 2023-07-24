#include "nes.hpp"

namespace nes
{

void mem_t::init()
{
    memset(data, 0x00, sizeof(data));
}

uint8_t mem_t::memory_read( uint16_t address )
{
    if ( address < 0x4000 || address >= 0x8000 )
    {
        return data[address];
    }

    return 0xFF;
}

void mem_t::memory_write( uint8_t value, uint16_t address )
{
    data[address] = value;
}

} // nes