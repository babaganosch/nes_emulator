#include "nes.hpp"

namespace nes
{

void mem_t::init()
{
    memset(data, 0x00, sizeof(data));
}

} // nes