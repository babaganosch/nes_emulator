#include "nes.hpp"

namespace nes
{

void mem_t::init()
{
    memset(data, 0x00, sizeof(data));
    data[0xC000] = 0x4C;
    data[0xC001] = 0xF5;
    data[0xC002] = 0xC5;
}

} // nes