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
    emu->ppu.cycles = 7 * 3;
    emu->ppu.x      = 7 * 3;

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
    uint16_t ppu_x = emu->ppu.x;
    uint16_t ppu_y = emu->ppu.y;

    char op_name[5];
    snprintf(op_name, 5, "%c%s", op.official == true ? ' ' : '*', op.name);

    post_fix_cursor  = 0;
    post_fix_letters = 0;

    if (op.addr_mode == addr_mode_implied)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X       %s                             A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, op_name, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
    }

    else if (op.addr_mode == addr_mode_absolute)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X %02X %s $%02X%02X ....                  A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, data1, op_name, data1, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 26;
        post_fix_letters = 4;
    }

    else if (op.addr_mode == addr_mode_immediate)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X    %s #$%02X                        A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op_name, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
    }

    else if (op.addr_mode == addr_mode_zero_page)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X    %s $%02X = ..                    A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op_name, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 26;
        post_fix_letters = 2;
    }

    else if (op.addr_mode == addr_mode_relative)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X    %s $....                       A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op_name, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 21;
        post_fix_letters = 4;
    }

    else if (op.addr_mode == addr_mode_accumulator)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X       %s A                           A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, op_name, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
    }

    else if (op.addr_mode == addr_mode_pre_index_indirect_x)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X    %s ($%02X,X) @ ..............    A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op_name, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 30;
        post_fix_letters = 14;
    }

    else if (op.addr_mode == addr_mode_post_index_indirect_y)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X    %s ($%02X),Y = ................  A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op_name, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 30;
        post_fix_letters = 16;
    }

    else if (op.addr_mode == addr_mode_indirect)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X %02X %s ($%02X%02X) = ....              A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, data1, op_name, data1, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 30;
        post_fix_letters = 4;
    }

    else if (op.addr_mode == addr_mode_index_y)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X %02X %s $%02X%02X,Y @ .........         A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, data1, op_name, data1, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 30;
        post_fix_letters = 9;
    }

    else if (op.addr_mode == addr_mode_index_x)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X %02X %s $%02X%02X,X @ .........         A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, data1, op_name, data1, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 30;
        post_fix_letters = 9;
    }

    else if (op.addr_mode == addr_mode_index_zp_x)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X    %s $%02X,X @ .......             A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op_name, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 28;
        post_fix_letters = 7;
    }

    else if (op.addr_mode == addr_mode_index_zp_y)
    {
        snprintf(emu_output, emu_output_len,
                 "%04X  %02X %02X    %s $%02X,Y @ .......             A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3u,%3u CYC:%u",
                 cpu.regs.PC, inst, data0, op_name, data0, cpu.regs.A, cpu.regs.X, cpu.regs.Y, cpu.regs.SR, cpu.regs.SP, ppu_y, ppu_x, cycles);
        post_fix_cursor  = 28;
        post_fix_letters = 7;
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
    if (strlen(emu->cpu.nestest_validation_str) < post_fix_letters)
    {
        printf("\033[0;31mERROR OCCOURED WHEN INJECTING DATA PEEK IN VALIDATION!\033[0;0m @ line: %u\n", line_number);
        return RESULT_ERROR;
    }
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
    const char* key_cline = line.c_str();

    if (strncmp(emu_output, key_cline, strlen(emu_output)) == 0)
    {   // OK!
        printf("     %-100s\033[0;32mOK!\033[0;0m\n", emu_output);
        validated_lines++;
    }
    else
    {   // FAILURE
        printf("\n-- \033[1;31mFAILED\033[0;0m --\n");
        printf(" @ line %u\n", line_number);
        printf("\033[0;34mRES:\033[0;0m ");
        auto i = 0;
        while( i < emu_output_len )
        {
            if ( i < strlen(key_cline) && i < strlen(emu_output) )
            {
                if (emu_output[i] == key_cline[i])
                {
                    printf("\033[0;0m%c", emu_output[i]);
                }
                else
                {
                    char c = emu_output[i] == ' ' ? '_' : emu_output[i];
                    printf("\033[0;31m%c", c);
                }
            }
            else
            {
                printf(" ");
            }
            ++i;
        }
        printf("    \033[0;31mFAIL!\033[0;0m\n");
        printf("\033[0;34mKEY:\033[0;0m %s\n", key_cline);
        return RESULT_ERROR;
    }

    if (key.peek() == '\n' || key.eof()) return RESULT_VALIDATION_SUCCESS;

    return RESULT_OK;
}


} // nes