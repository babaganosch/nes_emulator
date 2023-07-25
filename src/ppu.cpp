#include "nes.hpp"

namespace nes
{

RESULT ppu_t::init(mem_t &mem)
{
    memory = &mem;
    mem.cpu_mem.ppu_regs.PPUCTRL   = &regs.PPUCTRL;
    mem.cpu_mem.ppu_regs.PPUMASK   = &regs.PPUMASK;
    mem.cpu_mem.ppu_regs.PPUSTATUS = &regs.PPUSTATUS;
    mem.cpu_mem.ppu_regs.OAMADDR   = &regs.OAMADDR;
    mem.cpu_mem.ppu_regs.OAMDATA   = &regs.OAMDATA;
    mem.cpu_mem.ppu_regs.PPUSCROLL = &regs.PPUSCROLL;
    mem.cpu_mem.ppu_regs.PPUADDR   = &regs.PPUADDR;
    mem.cpu_mem.ppu_regs.PPUDATA   = &regs.PPUDATA;
    mem.cpu_mem.ppu_regs.OAMDMA    = &regs.OAMDMA;

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