#include "nes.hpp"
#include "nes_ops.hpp"
#include "logging.hpp"

#include <cstdio>

namespace nes
{

void cpu_t::init(cpu_callback_t cpu_cb, cpu_callback_t ppu_cb, cpu_callback_t apu_cb, mem_t* mem)
{
    cpu_callback = cpu_cb;
    ppu_callback = ppu_cb;
    apu_callback = apu_cb;
    memory = mem;
    memory->cpu = this;

    // Try to grab the interrupt vectors
    vectors.NMI = peek_short( 0xFFFA );
    vectors.RESET = peek_short( 0xFFFC );
    vectors.IRQBRK = peek_short( 0xFFFE );

    regs.SR = 0x24;
    regs.A  = regs.X = regs.Y = 0x00;
    regs.SP = 0xFD;
    regs.PC = vectors.RESET;

    cycles = 0u;

    trapped = false;
    nmi_pending = false;
    irq_pending = false;
}

static bool nmi = false;
uint16_t cpu_t::execute()
{
    // Reset CPU instruction delta
    delta_cycles = 0u;
    page_crossed = false;

    if (trapped) 
    { // JAM, CPU needs a restart
        tick_clock();
        return delta_cycles;
    }

    // Fetch instruction
    uint8_t old_ins = cur_ins;
    cur_ins = fetch_byte( regs.PC++ );
    // Check for interrupts (ignore IRQ if CLI followed by RTI)
    bool allow_irq = irq_pending && !(old_ins == 0x58 && cur_ins == 0x40);
    bool trig_nmi = nmi;

    // Perform instruction
    op_code_t& op_code = op_codes[cur_ins];
    op_code.function(*this, op_code.addr_mode );

    // Has IRQ/NMI occurred?
    if ( trig_nmi || (allow_irq && irq_inhibit == 0) ) {
        irq();
    }

    irq_inhibit = regs.I;
    return delta_cycles;
}

void cpu_t::tick_clock()
{
    delta_cycles++;
    cycles++;
    memory->cpu_cycles = cycles;

    if (cpu_callback)
    {
        cpu_callback(nullptr);
    }
    if (ppu_callback)
    { // NTSC PPU runs at 3x the CPU clock speed
        ppu_callback(nullptr);
        nmi |= nmi_pending;
        ppu_callback(nullptr);
        ppu_callback(nullptr);
        if (variant == PAL)
        { // PAL PPU runs at 3.2x the CPU clock speed
            pal_clock_buffer++;
            if (pal_clock_buffer >= 5)
            {
                ppu_callback(nullptr);
                pal_clock_buffer = 0;
            }
        }
    }
    if (apu_callback)
    {
        apu_callback(nullptr);
    }

}

void cpu_t::tick_clock( uint16_t ticks )
{
    while ( ticks-- > 0 )
    {
        tick_clock();
    }
}

void cpu_t::irq()
{ // Also NMI
    uint16_t vector = vectors.IRQBRK;
    push_byte_to_stack( (0xFF00 & regs.PC) >> 8 );
    push_byte_to_stack( 0x00FF & regs.PC );

    if (nmi)
    { // NMI hijack
        vector = vectors.NMI;
        nmi_pending = false;
        nmi = false;
    }

    push_byte_to_stack( regs.SR );    
    // Set I and fetch low nibble
    regs.I  = 1;
    regs.PC &= 0xFF00;
    regs.PC |= vector & 0x00FF;
    fetch_byte( vector == vectors.IRQBRK ? 0xFFFE : 0xFFFA );
    // Fetch high nibble
    regs.PC &= 0x00FF;
    regs.PC |= vector & 0xFF00;
    fetch_byte( vector == vectors.IRQBRK ? 0xFFFF : 0xFFFB );
}

uint8_t cpu_t::peek_byte( uint16_t address )
{
    return memory->memory_read( mem_t::CPU, address, true );
}

uint16_t cpu_t::peek_short( uint16_t address )
{
    uint16_t lo = memory->memory_read( mem_t::CPU, address,     true );
    uint16_t hi = memory->memory_read( mem_t::CPU, address + 1, true ) << 8;
    return hi | lo;
}

uint8_t cpu_t::fetch_byte( uint16_t address )
{
    uint8_t data = memory->memory_read( mem_t::CPU, address, false );
    tick_clock();
    return data;
}

uint8_t cpu_t::fetch_byte( uint8_t lo, uint8_t hi )
{
    uint16_t address = ((uint16_t)hi << 8) | lo;
    return fetch_byte( address );
}

uint8_t* cpu_t::fetch_byte_ref( uint16_t address )
{
    fetch_byte( address );
    return memory->fetch_byte_ref( address );
}

void cpu_t::write_byte( uint8_t data, uint8_t* ref )
{
    if (!ref)
    {
        LOG_E("CPU write byte to nullptr");
        throw RESULT_ERROR;
    }
    
    *ref = data;
    memory->cpu_mem.activity.read = false;
    tick_clock();
    memory->cpu_mem.activity.value = data;
    tick_clock();
}

void cpu_t::write_byte( uint8_t data, uint16_t address )
{
    memory->memory_write( mem_t::CPU, data, address );
    tick_clock();
}

void cpu_t::write_byte( uint8_t data, uint8_t lo, uint8_t hi )
{
    uint16_t address = ((uint16_t)hi << 8) | lo;
    memory->memory_write( mem_t::CPU, data, address );
    tick_clock();
}

void cpu_t::push_byte_to_stack( uint8_t data )
{
    if ( regs.SP == 0x00 )
    {
        //LOG_E("--- Stack Overflow! (cur ins: 0x%02X) ---", cur_ins);
        //throw;
    }
    uint8_t address = regs.SP--;
    memory->memory_write( mem_t::CPU, data, 0x100 + address );
    tick_clock();
}

void cpu_t::push_short_to_stack( uint16_t data )
{
    push_byte_to_stack( (0xFF00 & data) >> 8 );
    push_byte_to_stack( 0x00FF & data );
}

uint8_t cpu_t::pull_byte_from_stack( bool inc_sp )
{
    if ( regs.SP == 0xFF )
    {
        //LOG_E("--- Stack Underflow! (cur ins: 0x%02X) ---", cur_ins);
        //throw;
    }

    uint8_t data = fetch_byte(0x100 + regs.SP);
    if (inc_sp) regs.SP++;
    return data;
}

uint16_t cpu_t::pull_short_from_stack()
{
    uint8_t lo = pull_byte_from_stack( true );
    uint8_t hi = pull_byte_from_stack( false );
    return ((uint16_t)hi << 8) | lo;
}

void cpu_t::pre_inc_stack()
{
    fetch_byte( 0x100 + regs.SP++ );
}

} // nes