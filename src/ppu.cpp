#include "nes.hpp"

namespace nes
{

RESULT ppu_t::init()
{
    cycles = 0;
    x = 0;
    y = 0;
    return RESULT_OK;
}

RESULT ppu_t::execute()
{
    cycles++;
    x = cycles % 341;
    y = cycles / 341;
    return RESULT_OK;
}

} // nes