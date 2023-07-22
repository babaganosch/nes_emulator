#ifndef NES_OPS_H
#define NES_OPS_H

#include <cstdint>

#include "nes.hpp"

namespace nes
{

namespace
{
#define CPU_OP(OPC, ADDR_MODE) {       \
    .addr_mode = addr_mode_##ADDR_MODE, \
    .function = OP_##OPC, .name = #OPC }
#define ADDRESS_MODE(MODE) uint16_t addr_mode_##MODE(cpu_t &cpu, bool modify_memory)
#define OP_FUNCTION(NAME) void OP_##NAME(cpu_t &cpu, addr_mode_t addr_mode)
} // anonymous

typedef uint16_t (* addr_mode_t)(cpu_t &cpu, bool modify_memory);
typedef void (* op_code_function_t)(cpu_t &cpu, addr_mode_t addr_mode);

struct op_code_t
{
    addr_mode_t addr_mode;
    op_code_function_t function;
    const char name[4];
};


// addr modes
ADDRESS_MODE(implied);               //
ADDRESS_MODE(immediate);             // _const  imm = #$00
ADDRESS_MODE(absolute);              // _abs    abs = $0000
ADDRESS_MODE(zero_page);             // _zp      zp = $00
ADDRESS_MODE(index_x);               // _abs_x  abx = $0000,X
ADDRESS_MODE(index_y);               // _abs_y  aby = $0000,Y
ADDRESS_MODE(index_zp_x);            // _zp_x   zpx = $00,X
ADDRESS_MODE(index_zp_y);            // _zp_y   zpy = $00,Y
ADDRESS_MODE(indirect);              // _ind    ind = ($0000)
ADDRESS_MODE(pre_index_indirect_x);  // _ind_x  izx = ($00,X)
ADDRESS_MODE(post_index_indirect_y); // _ind_y  izy = ($00),Y
ADDRESS_MODE(relative);              //         rel = $0000
ADDRESS_MODE(accumulator);           // A       

// OPS
OP_FUNCTION(___); // Not Implemented OP

OP_FUNCTION(ADC);
OP_FUNCTION(AND);
OP_FUNCTION(ASL);

// Not sorted bellow this line
OP_FUNCTION(JMP);
OP_FUNCTION(LDX);
OP_FUNCTION(STX);
OP_FUNCTION(JSR);
OP_FUNCTION(NOP);
OP_FUNCTION(LDA);
OP_FUNCTION(SEC);
OP_FUNCTION(BCS);
OP_FUNCTION(CLC);
OP_FUNCTION(BCC);
OP_FUNCTION(BEQ);
OP_FUNCTION(BNE);
OP_FUNCTION(STA);
OP_FUNCTION(BIT);
OP_FUNCTION(BVC);
OP_FUNCTION(BVS);
OP_FUNCTION(BPL);
OP_FUNCTION(RTS);
OP_FUNCTION(SEI);
OP_FUNCTION(SED);
OP_FUNCTION(PHP);
OP_FUNCTION(PLA);
OP_FUNCTION(CMP);
OP_FUNCTION(CLD);
OP_FUNCTION(PHA);
OP_FUNCTION(PLP);
OP_FUNCTION(BMI);
OP_FUNCTION(ORA);

extern op_code_t op_codes[256];

} // nes

#endif /* NES_OPS_H */