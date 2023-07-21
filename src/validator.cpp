#include "nes_validator.hpp"
#include "nes_ops.hpp"
#include "nes.hpp"

namespace nes
{

validator::~validator()
{
    if (key.is_open())
    {
        key.close();
    }
}

RESULT validator::init(emu_t* emu_ref, const char* key_path)
{
    emu = emu_ref;
    emu->cpu.nestest_validation = true;
    emu->cpu.cycles = 7;

    key.open(key_path, std::ios::in |  std::ios::ate);
    auto file_size = key.tellg();
    key.seekg(0, key.beg);
    if (!key.good() || file_size == 0)
    {
        printf("Failed to open '%s'\n", key_path);
        return RESULT_ERROR;
    }

    return RESULT_OK;
}

RESULT validator::execute()
{
    RESULT ret = construct_output_pre_line();
    if (ret != RESULT_OK) return ret;

    // Clear validation str
    *emu->cpu.nestest_validation_str = '\0';

    ret = emu->step(1);
    if (ret != RESULT_OK) return ret; 

    ret = construct_output_post_line();
    if (ret != RESULT_OK) return ret; 

    ret = validate_line();

    return ret;
}

RESULT validator::construct_output_pre_line()
{
    line_number++;
    emu_output[0] = '\0';
    cpu_t& cpu    = emu->cpu;
    uint8_t inst  = emu->memory[cpu.regs.PC    ];
    uint8_t data0 = emu->memory[cpu.regs.PC + 1];
    uint8_t data1 = emu->memory[cpu.regs.PC + 2];
    op_code_t op  = op_codes[inst];
    uint16_t cycles = cpu.cycles;
    // TODO(PPU)
    uint16_t ppu_x = (cycles * 3) % 341, ppu_y = (cycles / 341);

    post_fix_cursor  = 0;
    post_fix_letters = 0;

    if (op.addr_mode.function == addr_mode_absolute)
    {
        snprintf(emu_output, 95, "%04X  %02X %02X %02X  %s $%02X%02X                       A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, data1, op.name, data1, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
    }

    else if (op.addr_mode.function == addr_mode_immediate)
    {
        snprintf(emu_output, 95, "%04X  %02X %02X     %s #$%02X                        A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op.name, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
    }

    else if (op.addr_mode.function == addr_mode_zero_page)
    {
        snprintf(emu_output, 95, "%04X  %02X %02X     %s $%02X = ..                    A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op.name, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 26;
        post_fix_letters = 2;
    }

    else
    {
        printf("UNIMPLEMENTED VALIDATION FOR LINE %u    OP-CODE: %02X (%s)\n", line_number, inst, op.name);
        return RESULT_ERROR;
    }

    return RESULT_OK;
}

RESULT validator::construct_output_post_line()
{
    auto index = 0;
    while (post_fix_letters > 0)
    {
        emu_output[post_fix_cursor++] = emu->cpu.nestest_validation_str[index++];
        post_fix_letters--;
    }

    return RESULT_OK;
}

RESULT validator::validate_line()
{
    std::string line;
    std::getline(key, line);

    printf("RES: %s\n", emu_output);
    printf("KEY: %s\n", line.c_str());

    return RESULT_OK;
}


} // nes