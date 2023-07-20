#ifndef NES_OPS_H
#define NES_OPS_H

#include <cstdint>

#include "nes.hpp"

namespace nes
{

namespace
{
#define CPU_OP(ADDR_MODE, OPC) { .addr_mode = addr_mode_##ADDR_MODE, .function = OP_##OPC }
#define ADDRESS_MODE(MODE) uint8_t* addr_mode_##MODE(cpu_t &cpu, bool modify)
#define OP_FUNCTION(NAME) void OP_##NAME(cpu_t &cpu, addr_mode_t &addr_mode)
} // anonymous

typedef uint8_t* (* addr_mode_t)(cpu_t &cpu, bool modify);
typedef void (* op_code_function_t)(cpu_t &cpu, addr_mode_t &addr_mode);

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
OP_FUNCTION(UNIMPLEMENTED);
OP_FUNCTION(ADC);
OP_FUNCTION(AND);
OP_FUNCTION(ASL);

OP_FUNCTION(LDA);

struct op_code_t
{
    addr_mode_t addr_mode;
    op_code_function_t function;
};

extern op_code_t op_codes[256];

} // nes

#endif /* NES_OPS_H */