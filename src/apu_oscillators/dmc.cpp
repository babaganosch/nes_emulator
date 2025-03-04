#include "logging.hpp"
#include "nes.hpp"

namespace nes
{

constexpr uint16_t period_lut[16] = { // Periods in CPU Cycles
    0x1AC, 0x17C, 0x154, 0x140, 0x11E, 0xFE, 0xE2, 0xD6, 0xBE, 0xA0, 0x8E, 0x80, 0x6A, 0x54, 0x48, 0x36 // NTSC: 0x00 - 0x0F 
};

void apu_t::dmc_t::write( uint16_t address, uint8_t value )
{
    switch ( address )
    {
        case ( 0x4010 ): 
        { // Flags and rate
            control.data = value;
            if (!control.irq_enable) interrupt_flag = false;
            period = period_lut[control.rate]; // 0x0 - 0xF NTSC
            //LOG_I("4010 control %02X (rate: 0x%03X) (I:%u L:%u)", value, period, control.irq_enable, control.loop);
        } break;
        case ( 0x4011 ): 
        { // Direct load
            direct_load.data = value;
            output_level = direct_load.load;
        } break;
        case ( 0x4012 ): 
        { // Sample address
            sample_address = value;
        } break;
        case ( 0x4013 ): 
        { // Sample length
            sample_length = value;
        } break;
    }
}

void apu_t::dmc_t::tick()
{
    memory_reader.tick( this );

    if (++timer > period)
    { // Advance
        timer = 0;
        // 1.
        if (!silence)
        {
            if (bits_shift_register & 1)
            { // Increase output level
                if (output_level < 127) output_level += 2;
            } else
            { // Decrease output level
                if (output_level > 0) output_level -= 2;
            }
        }

        // 2
        bits_shift_register >>= 1;

        // 3.
        if (bits_remaining_register > 0)
        {
            bits_remaining_register--;
        }
        if (bits_remaining_register == 0)
        {
            bits_remaining_register = 8;
            if (sample_buffer.empty)
            {
                silence = true;
            } else
            {
                silence = false;
                bits_shift_register = sample_buffer.data;
                sample_buffer.empty = true;
            }
        }
    }
    get_cycle = !get_cycle;
    put_cycle = !put_cycle;
}

void apu_t::dmc_t::memory_reader_t::start_sample( dmc_t* dmc )
{
    bytes_remaining_counter = (dmc->sample_length * 16) + 1;
    address_counter = 0;
    dmc->playing = true;
}

void apu_t::dmc_t::memory_reader_t::tick( dmc_t* dmc )
{
    if (bytes_remaining_counter > 0 && dmc->playing)
    {
        if (dmc->sample_buffer.empty) 
        { // Buffer is empty, load it!
            if (dmc->get_cycle && !data_loaded)
            { // Correct cycle to get data, do it!
                uint16_t address = 0xC000 + (dmc->sample_address * 64) + address_counter;
                address += address < 0xC000 ? 0x8000 : 0; // Wrap around to 0x8000
                data_loaded = true;
                // TODO: Need to stall the CPU
                tmp_data = memory->memory_read(nes::mem_t::CPU, address, true);
                memory->cpu->dma_halt_cycles = 1;
                LOG_D("DMC DMA READ: 0x%02X 0x%04X %u", tmp_data, address, bytes_remaining_counter);
            } 
            else if (dmc->put_cycle && data_loaded)
            {
                dmc->sample_buffer.data = tmp_data;
                dmc->sample_buffer.empty = false;
                data_loaded = false;
                address_counter++;
                bytes_remaining_counter--;
            }

            if (bytes_remaining_counter == 0)
            {
                dmc->playing = false;
                if (dmc->control.loop)
                { // The sample is restarted (see above);
                    start_sample( dmc );
                } else if (dmc->control.irq_enable)
                { // If the IRQ enabled flag is set, the interrupt flag is set.
                    dmc->interrupt_flag = true;
                }
                dmc->play = false;
            }

            
        }
    } 
    else if (dmc->play) {
        start_sample( dmc );
    }
}



} // nes