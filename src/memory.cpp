#include "nes.hpp"

namespace nes
{

void mem_t::init()
{
    memset(data, 0x02, sizeof(data));
}

} // nes