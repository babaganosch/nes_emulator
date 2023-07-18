#ifndef NES_OPS_H
#define NES_OPS_H

#include <cstdint>

#include "nes.hpp"

namespace nes
{

namespace
{
#define CPU_OP(ADDR_MODE, OPC) { .addr_mode = addr_mode_##ADDR_MODE, .function = OPC }
}

typedef uint16_t (* addr_mode_t)(uint8_t lo, uint8_t hi);
typedef void (* op_code_function_t)(cpu_t &cpu, mem_t *mem, uint8_t data);

// addr modes
uint16_t addr_mode_absolute(uint8_t lo, uint8_t hi) { printf("Addr: absolute\n"); return 0; };
uint16_t addr_mode_implied(uint8_t _, uint8_t __)   { printf("Addr: implied\n");  return 10; };
//...

// OPS
void LDA(cpu_t &cpu, mem_t *mem, uint8_t data) { printf("LDA\n"); };
void LDX(cpu_t &cpu, mem_t *mem, uint8_t data) { printf("LDX\n"); };
//...

struct op_codes_t
{
    addr_mode_t addr_mode;
    op_code_function_t function;
};

op_codes_t op_codes[3] = {
    //{ .addr_mode = addr_mode_absolute, .function = LDA }
    
    CPU_OP(absolute, LDA), // 1
    CPU_OP(implied,  LDA), // 2
    CPU_OP(absolute, LDX)  // 3
};




} // nes

#endif /* NES_OPS_H */