#include "nes.hpp"

namespace nes
{

void mem_t::init()
{
    memset(data, 0xFF, sizeof(data));
    data[0xC000] = 0xAD;
}

} // nes