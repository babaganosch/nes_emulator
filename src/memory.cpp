#include "nes.hpp"

namespace nes
{

void mem_t::init()
{
    for (auto i = 0; i < sizeof(data); ++i)
    {
        data[i] = 0u;
    }
}

} // nes