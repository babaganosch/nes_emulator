#include "nes.hpp"

namespace nes
{

void mem_t::init()
{
    memset(data, 0xAD, sizeof(data));
}

} // nes