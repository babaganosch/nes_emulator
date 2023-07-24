#ifndef NES_OPS_H
#define NES_OPS_H

#include <cstdint>

#include "nes.hpp"

namespace nes
{

namespace
{
#define CPU_OP(OPC, OFFICIAL, ADDR_MODE)   \
{                                          \
    .addr_mode = addr_mode_##ADDR_MODE,    \
    .function  = OP_##OPC,                 \
    .official  = OFFICIAL,                 \
    .name      = #OPC                      \
}
#define ADDRESS_MODE(MODE) uint16_t addr_mode_##MODE(cpu_t &cpu, bool modify_memory, bool is_branch)
#define OP_FUNCTION(NAME) void OP_##NAME(cpu_t &cpu, addr_mode_t addr_mode)
} // anonymous

typedef uint16_t (* addr_mode_t)(cpu_t &cpu, bool modify_memory, bool is_branch);
typedef void (* op_code_function_t)(cpu_t &cpu, addr_mode_t addr_mode);

struct op_code_t
{
    addr_mode_t addr_mode;
    op_code_function_t function;
    const bool official;
    const char name[4];
};

// Addressing Modes
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

/// Official OPs
OP_FUNCTION(ADC);
OP_FUNCTION(AND);
OP_FUNCTION(ASL);
OP_FUNCTION(BCC);
OP_FUNCTION(BCS);
OP_FUNCTION(BEQ);
OP_FUNCTION(BIT);
OP_FUNCTION(BMI);
OP_FUNCTION(BNE);
OP_FUNCTION(BPL);
OP_FUNCTION(BRK);
OP_FUNCTION(BVC);
OP_FUNCTION(BVS);
OP_FUNCTION(CLC);
OP_FUNCTION(CLD);
OP_FUNCTION(CLI);
OP_FUNCTION(CLV);
OP_FUNCTION(CMP);
OP_FUNCTION(CPX);
OP_FUNCTION(CPY);
OP_FUNCTION(DEC);
OP_FUNCTION(DEX);
OP_FUNCTION(DEY);
OP_FUNCTION(EOR);
OP_FUNCTION(INC);
OP_FUNCTION(INX);
OP_FUNCTION(INY);
OP_FUNCTION(JMP);
OP_FUNCTION(JSR);
OP_FUNCTION(LDA);
OP_FUNCTION(LDX);
OP_FUNCTION(LDY);
OP_FUNCTION(LSR);
OP_FUNCTION(NOP);
OP_FUNCTION(ORA);
OP_FUNCTION(PHA);
OP_FUNCTION(PHP);
OP_FUNCTION(PLA);
OP_FUNCTION(PLP);
OP_FUNCTION(ROL);
OP_FUNCTION(ROR);
OP_FUNCTION(RTI);
OP_FUNCTION(RTS);
OP_FUNCTION(SBC);
OP_FUNCTION(SEC);
OP_FUNCTION(SED);
OP_FUNCTION(SEI);
OP_FUNCTION(STA);
OP_FUNCTION(STX);
OP_FUNCTION(STY);
OP_FUNCTION(TAX);
OP_FUNCTION(TAY);
OP_FUNCTION(TSX);
OP_FUNCTION(TXA);
OP_FUNCTION(TXS);
OP_FUNCTION(TYA);

/// Illegal OPs   -- aliases
OP_FUNCTION(LAX); //
OP_FUNCTION(SAX); // AXS, AAX
OP_FUNCTION(DCP); // DCM
OP_FUNCTION(ISB); // ISC, INS
OP_FUNCTION(SLO); // ASO
OP_FUNCTION(RLA); //
OP_FUNCTION(SRE); // LSE
OP_FUNCTION(RRA); // 

/// Not Implemented OP
OP_FUNCTION(___);

extern op_code_t op_codes[256];

} // nes

#endif /* NES_OPS_H */