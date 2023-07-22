#include "nes_ops.hpp"
#include "nes.hpp"

#define CALC_Z_FLAG(VALUE) cpu.regs.Z = (((VALUE & 0xFF) == 0x00) ? 1 : 0 )
#define CALC_N_FLAG(VALUE) cpu.regs.N = (((VALUE & 0x80) > 0x00) ? 1 : 0 )
#define CALC_C_FLAG(VALUE) cpu.regs.C = (( VALUE > 0xFF ) ? 1 : 0 )
#define CALC_V_FLAG(M, N, RESULT) cpu.regs.V = ((((M ^ RESULT) & (N ^ RESULT) & 0x80) != 0) ? 1 : 0 )

#define UINT16(LO, HI) (((uint16_t) HI << 8) | LO)

namespace nes
{

namespace
{
void branch( cpu_t &cpu, uint8_t condition, uint8_t expected, uint16_t PC, uint16_t target )
{
    if (condition == expected)
    { // Branch occurs
        cpu.tick_clock();
        if ( (PC & 0xFF00) != (PC & 0xFF00) )
        { // Branch caused page change
            cpu.tick_clock();
        }
        cpu.regs.PC = target;
    }
}
} // anonymous

op_code_t op_codes[256] = {
    CPU_OP(___, implied),                //   0     $ 00
    CPU_OP(___, implied),                //   1     $ 01
    CPU_OP(___, implied),                //   2     $ 02
    CPU_OP(___, implied),                //   3     $ 03
    CPU_OP(___, implied),                //   4     $ 04
    CPU_OP(___, implied),                //   5     $ 05
    CPU_OP(ASL, zero_page),              //   6     $ 06
    CPU_OP(___, implied),                //   7     $ 07
    CPU_OP(PHP, implied),                //   8     $ 08
    CPU_OP(___, implied),                //   9     $ 09
    CPU_OP(ASL, accumulator),            //  10     $ 0A
    CPU_OP(___, implied),                //  11     $ 0B
    CPU_OP(___, implied),                //  12     $ 0C
    CPU_OP(___, implied),                //  13     $ 0D
    CPU_OP(ASL, absolute),               //  14     $ 0E
    CPU_OP(___, implied),                //  15     $ 0F
    CPU_OP(BPL, relative),               //  16     $ 10
    CPU_OP(___, implied),                //  17     $ 11
    CPU_OP(___, implied),                //  18     $ 12
    CPU_OP(___, implied),                //  19     $ 13
    CPU_OP(___, implied),                //  20     $ 14
    CPU_OP(___, implied),                //  21     $ 15
    CPU_OP(ASL, index_zp_x),             //  22     $ 16
    CPU_OP(___, implied),                //  23     $ 17
    CPU_OP(CLC, implied),                //  24     $ 18
    CPU_OP(___, implied),                //  25     $ 19
    CPU_OP(___, implied),                //  26     $ 1A
    CPU_OP(___, implied),                //  27     $ 1B
    CPU_OP(___, implied),                //  28     $ 1C
    CPU_OP(___, implied),                //  29     $ 1D
    CPU_OP(ASL, index_x),                //  30     $ 1E
    CPU_OP(___, implied),                //  31     $ 1F
    CPU_OP(JSR, absolute),               //  32     $ 20
    CPU_OP(AND, pre_index_indirect_x),   //  33     $ 21
    CPU_OP(___, implied),                //  34     $ 22
    CPU_OP(___, implied),                //  35     $ 23
    CPU_OP(BIT, zero_page),              //  36     $ 24
    CPU_OP(AND, zero_page),              //  37     $ 25
    CPU_OP(___, implied),                //  38     $ 26
    CPU_OP(___, implied),                //  39     $ 27
    CPU_OP(___, implied),                //  40     $ 28
    CPU_OP(AND, immediate),              //  41     $ 29
    CPU_OP(___, implied),                //  42     $ 2A
    CPU_OP(___, implied),                //  43     $ 2B
    CPU_OP(BIT, absolute),               //  44     $ 2C
    CPU_OP(AND, absolute),               //  45     $ 2D
    CPU_OP(___, implied),                //  46     $ 2E
    CPU_OP(___, implied),                //  47     $ 2F
    CPU_OP(___, implied),                //  48     $ 30
    CPU_OP(AND, post_index_indirect_y),  //  49     $ 31
    CPU_OP(___, implied),                //  50     $ 32
    CPU_OP(___, implied),                //  51     $ 33
    CPU_OP(___, implied),                //  52     $ 34
    CPU_OP(AND, index_zp_x),             //  53     $ 35
    CPU_OP(___, implied),                //  54     $ 36
    CPU_OP(___, implied),                //  55     $ 37
    CPU_OP(SEC, implied),                //  56     $ 38
    CPU_OP(AND, index_y),                //  57     $ 39
    CPU_OP(___, implied),                //  58     $ 3A
    CPU_OP(___, implied),                //  59     $ 3B
    CPU_OP(___, implied),                //  60     $ 3C
    CPU_OP(AND, index_x),                //  61     $ 3D
    CPU_OP(___, implied),                //  62     $ 3E
    CPU_OP(___, implied),                //  63     $ 3F
    CPU_OP(___, implied),                //  64     $ 40
    CPU_OP(___, implied),                //  65     $ 41
    CPU_OP(___, implied),                //  66     $ 42
    CPU_OP(___, implied),                //  67     $ 43
    CPU_OP(___, implied),                //  68     $ 44
    CPU_OP(___, implied),                //  69     $ 45
    CPU_OP(___, implied),                //  70     $ 46
    CPU_OP(___, implied),                //  71     $ 47
    CPU_OP(___, implied),                //  72     $ 48
    CPU_OP(___, implied),                //  73     $ 49
    CPU_OP(___, implied),                //  74     $ 4A
    CPU_OP(___, implied),                //  75     $ 4B
    CPU_OP(JMP, absolute),               //  76     $ 4C
    CPU_OP(___, implied),                //  77     $ 4D
    CPU_OP(___, implied),                //  78     $ 4E
    CPU_OP(___, implied),                //  79     $ 4F
    CPU_OP(BVC, relative),               //  80     $ 50
    CPU_OP(___, implied),                //  81     $ 51
    CPU_OP(___, implied),                //  82     $ 52
    CPU_OP(___, implied),                //  83     $ 53
    CPU_OP(___, implied),                //  84     $ 54
    CPU_OP(___, implied),                //  85     $ 55
    CPU_OP(___, implied),                //  86     $ 56
    CPU_OP(___, implied),                //  87     $ 57
    CPU_OP(___, implied),                //  88     $ 58
    CPU_OP(___, implied),                //  89     $ 59
    CPU_OP(___, implied),                //  90     $ 5A
    CPU_OP(___, implied),                //  91     $ 5B
    CPU_OP(___, implied),                //  92     $ 5C
    CPU_OP(___, implied),                //  93     $ 5D
    CPU_OP(___, implied),                //  94     $ 5E
    CPU_OP(___, implied),                //  95     $ 5F
    CPU_OP(RTS, implied),                //  96     $ 60
    CPU_OP(ADC, pre_index_indirect_x),   //  97     $ 61
    CPU_OP(___, implied),                //  98     $ 62
    CPU_OP(___, implied),                //  99     $ 63
    CPU_OP(___, implied),                // 100     $ 64
    CPU_OP(ADC, zero_page),              // 101     $ 65
    CPU_OP(___, implied),                // 102     $ 66
    CPU_OP(___, implied),                // 103     $ 67
    CPU_OP(PLA, implied),                // 104     $ 68
    CPU_OP(ADC, immediate),              // 105     $ 69
    CPU_OP(___, implied),                // 106     $ 6A
    CPU_OP(___, implied),                // 107     $ 6B
    CPU_OP(JMP, indirect),               // 108     $ 6C
    CPU_OP(ADC, absolute),               // 109     $ 6D
    CPU_OP(___, implied),                // 110     $ 6E
    CPU_OP(___, implied),                // 111     $ 6F
    CPU_OP(BVS, relative),               // 112     $ 70
    CPU_OP(ADC, post_index_indirect_y),  // 113     $ 71
    CPU_OP(___, implied),                // 114     $ 72
    CPU_OP(___, implied),                // 115     $ 73
    CPU_OP(___, implied),                // 116     $ 74
    CPU_OP(ADC, index_zp_x),             // 117     $ 75
    CPU_OP(___, implied),                // 118     $ 76
    CPU_OP(___, implied),                // 119     $ 77
    CPU_OP(SEI, implied),                // 120     $ 78
    CPU_OP(ADC, index_y),                // 121     $ 79
    CPU_OP(___, implied),                // 122     $ 7A
    CPU_OP(___, implied),                // 123     $ 7B
    CPU_OP(___, implied),                // 124     $ 7C
    CPU_OP(ADC, index_x),                // 125     $ 7D
    CPU_OP(___, implied),                // 126     $ 7E
    CPU_OP(___, implied),                // 127     $ 7F
    CPU_OP(___, implied),                // 128     $ 80
    CPU_OP(STA, pre_index_indirect_x),   // 129     $ 81
    CPU_OP(___, implied),                // 130     $ 82
    CPU_OP(___, implied),                // 131     $ 83
    CPU_OP(___, implied),                // 132     $ 84
    CPU_OP(STA, zero_page),              // 133     $ 85
    CPU_OP(STX, zero_page),              // 134     $ 86
    CPU_OP(___, implied),                // 135     $ 87
    CPU_OP(___, implied),                // 136     $ 88
    CPU_OP(___, implied),                // 137     $ 89
    CPU_OP(___, implied),                // 138     $ 8A
    CPU_OP(___, implied),                // 139     $ 8B
    CPU_OP(___, implied),                // 140     $ 8C
    CPU_OP(STA, absolute),               // 141     $ 8D
    CPU_OP(STX, absolute),               // 142     $ 8E
    CPU_OP(___, implied),                // 143     $ 8F
    CPU_OP(BCC, relative),               // 144     $ 90
    CPU_OP(STA, post_index_indirect_y),  // 145     $ 91
    CPU_OP(___, implied),                // 146     $ 92
    CPU_OP(___, implied),                // 147     $ 93
    CPU_OP(___, implied),                // 148     $ 94
    CPU_OP(STA, index_zp_x),             // 149     $ 95
    CPU_OP(STX, index_zp_y),             // 150     $ 96
    CPU_OP(___, implied),                // 151     $ 97
    CPU_OP(___, implied),                // 152     $ 98
    CPU_OP(STA, index_y),                // 153     $ 99
    CPU_OP(___, implied),                // 154     $ 9A
    CPU_OP(___, implied),                // 155     $ 9B
    CPU_OP(___, implied),                // 156     $ 9C
    CPU_OP(STA, index_x),                // 157     $ 9D
    CPU_OP(___, implied),                // 158     $ 9E
    CPU_OP(___, implied),                // 159     $ 9F
    CPU_OP(___, implied),                // 160     $ A0
    CPU_OP(___, implied),                // 161     $ A1
    CPU_OP(LDX, immediate),              // 162     $ A2
    CPU_OP(___, implied),                // 163     $ A3
    CPU_OP(___, implied),                // 164     $ A4
    CPU_OP(LDA, zero_page),              // 165     $ A5
    CPU_OP(LDX, zero_page),              // 166     $ A6
    CPU_OP(___, implied),                // 167     $ A7
    CPU_OP(___, implied),                // 168     $ A8
    CPU_OP(LDA, immediate),              // 169     $ A9
    CPU_OP(___, implied),                // 170     $ AA
    CPU_OP(___, implied),                // 171     $ AB
    CPU_OP(___, implied),                // 172     $ AC
    CPU_OP(LDA, absolute),               // 173     $ AD
    CPU_OP(LDX, absolute),               // 174     $ AE
    CPU_OP(___, implied),                // 175     $ AF
    CPU_OP(BCS, relative),               // 176     $ B0
    CPU_OP(LDA, post_index_indirect_y),  // 177     $ B1
    CPU_OP(___, implied),                // 178     $ B2
    CPU_OP(___, implied),                // 179     $ B3
    CPU_OP(___, implied),                // 180     $ B4
    CPU_OP(LDA, index_zp_x),             // 181     $ B5
    CPU_OP(LDX, index_zp_x),             // 182     $ B6
    CPU_OP(___, implied),                // 183     $ B7
    CPU_OP(___, implied),                // 184     $ B8
    CPU_OP(LDA, index_y),                // 185     $ B9
    CPU_OP(___, implied),                // 186     $ BA
    CPU_OP(___, implied),                // 187     $ BB
    CPU_OP(___, implied),                // 188     $ BC
    CPU_OP(LDA, index_x),                // 189     $ BD
    CPU_OP(LDX, index_y),                // 190     $ BE
    CPU_OP(___, implied),                // 191     $ BF
    CPU_OP(___, implied),                // 192     $ C0
    CPU_OP(CMP, pre_index_indirect_x),   // 193     $ C1
    CPU_OP(___, implied),                // 194     $ C2
    CPU_OP(___, implied),                // 195     $ C3
    CPU_OP(___, implied),                // 196     $ C4
    CPU_OP(CMP, zero_page),              // 197     $ C5
    CPU_OP(___, implied),                // 198     $ C6
    CPU_OP(___, implied),                // 199     $ C7
    CPU_OP(___, implied),                // 200     $ C8
    CPU_OP(CMP, immediate),              // 201     $ C9
    CPU_OP(___, implied),                // 202     $ CA
    CPU_OP(___, implied),                // 203     $ CB
    CPU_OP(___, implied),                // 204     $ CC
    CPU_OP(CMP, absolute),               // 205     $ CD
    CPU_OP(___, implied),                // 206     $ CE
    CPU_OP(___, implied),                // 207     $ CF
    CPU_OP(BNE, relative),               // 208     $ D0
    CPU_OP(CMP, post_index_indirect_y),  // 209     $ D1
    CPU_OP(___, implied),                // 210     $ D2
    CPU_OP(___, implied),                // 211     $ D3
    CPU_OP(___, implied),                // 212     $ D4
    CPU_OP(CMP, index_zp_x),             // 213     $ D5
    CPU_OP(___, implied),                // 214     $ D6
    CPU_OP(___, implied),                // 215     $ D7
    CPU_OP(___, implied),                // 216     $ D8
    CPU_OP(CMP, index_y),                // 217     $ D9
    CPU_OP(___, implied),                // 218     $ DA
    CPU_OP(___, implied),                // 219     $ DB
    CPU_OP(___, implied),                // 220     $ DC
    CPU_OP(CMP, index_x),                // 221     $ DD
    CPU_OP(___, implied),                // 222     $ DE
    CPU_OP(___, implied),                // 223     $ DF
    CPU_OP(___, implied),                // 224     $ E0
    CPU_OP(___, implied),                // 225     $ E1
    CPU_OP(___, implied),                // 226     $ E2
    CPU_OP(___, implied),                // 227     $ E3
    CPU_OP(___, implied),                // 228     $ E4
    CPU_OP(___, implied),                // 229     $ E5
    CPU_OP(___, implied),                // 230     $ E6
    CPU_OP(___, implied),                // 231     $ E7
    CPU_OP(___, implied),                // 232     $ E8
    CPU_OP(___, implied),                // 233     $ E9
    CPU_OP(NOP, implied),                // 234     $ EA
    CPU_OP(___, implied),                // 235     $ EB
    CPU_OP(___, implied),                // 236     $ EC
    CPU_OP(___, implied),                // 237     $ ED
    CPU_OP(___, implied),                // 238     $ EE
    CPU_OP(___, implied),                // 239     $ EF
    CPU_OP(BEQ, relative),               // 240     $ F0
    CPU_OP(___, implied),                // 241     $ F1
    CPU_OP(___, implied),                // 242     $ F2
    CPU_OP(___, implied),                // 243     $ F3
    CPU_OP(___, implied),                // 244     $ F4
    CPU_OP(___, implied),                // 245     $ F5
    CPU_OP(___, implied),                // 246     $ F6
    CPU_OP(___, implied),                // 247     $ F7
    CPU_OP(SED, implied),                // 248     $ F8
    CPU_OP(___, implied),                // 249     $ F9
    CPU_OP(___, implied),                // 250     $ FA
    CPU_OP(___, implied),                // 251     $ FB
    CPU_OP(___, implied),                // 252     $ FC
    CPU_OP(___, implied),                // 253     $ FD
    CPU_OP(___, implied),                // 254     $ FE
    CPU_OP(___, implied)                 // 255     $ FF
};          

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Addressing modes

ADDRESS_MODE(implied)
{
    return 0;
}

ADDRESS_MODE(immediate)
{
    return cpu.regs.PC++;
}

ADDRESS_MODE(absolute)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    return UINT16( lo, hi );
}

ADDRESS_MODE(zero_page)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint16_t address = UINT16( lo, 0x00 );
    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 3, "%02X", cpu.peek_memory(address));
    }
    return address;
}

ADDRESS_MODE(index_x)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t new_lo = cpu.regs.X + lo;
    if ( new_lo < lo || modify_memory )
    { // Page crossing
        hi++;
        cpu.tick_clock();
    }
    return UINT16( new_lo, hi );
}

ADDRESS_MODE(index_y)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t new_lo = cpu.regs.Y + lo;
    if ( new_lo < lo || modify_memory )
    { // Page crossing
        hi++;
        cpu.tick_clock();
    }
    return UINT16( new_lo, hi );
}

ADDRESS_MODE(index_zp_x)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    lo += cpu.regs.X;
    return UINT16( lo, 0x00 );
}

ADDRESS_MODE(index_zp_y)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    lo += cpu.regs.Y;
    return UINT16( lo, 0x00 );
}

ADDRESS_MODE(indirect)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    uint16_t new_address = UINT16( lo, hi );
    lo = cpu.fetch_byte( new_address     );
    hi = cpu.fetch_byte( new_address + 1 );
    return UINT16( lo, hi );
}

ADDRESS_MODE(pre_index_indirect_x)
{
    uint8_t tmp = cpu.fetch_byte( cpu.regs.PC++ );
    tmp += cpu.regs.X;
    uint8_t lo = cpu.fetch_byte( tmp,     0x00 );
    uint8_t hi = cpu.fetch_byte( tmp + 1, 0x00 );
    return UINT16( lo, hi );
}

ADDRESS_MODE(post_index_indirect_y)
{
    uint8_t tmp = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t lo  = cpu.fetch_byte( tmp,     0x00 );
    uint8_t hi  = cpu.fetch_byte( tmp + 1, 0x00 );
    uint8_t new_lo = lo + cpu.regs.Y;
    if ( new_lo < lo || modify_memory )
    { // Page crossing
        hi++;
        cpu.tick_clock();
    }
    return UINT16( new_lo, hi );
}

ADDRESS_MODE(relative)
{
    // TODO(xxx): Remember to add extra clock cycle if branch
    //            causes page movement
    int8_t offset = cpu.fetch_byte( cpu.regs.PC++ );
    uint16_t address = cpu.regs.PC + offset;
    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 5, "%04X", address);
    }
    return address;
}

ADDRESS_MODE(accumulator)
{
    return cpu.regs.A;
}

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// OPs
OP_FUNCTION(___)
{
    printf("UNIMPLEMENTED OP-CODE ENCOUNTERED!\n");
}

/////////////////////////////////////////////////////////
// ADC - Add Memory to Accumulator with Carry
// A + M + C -> A, C
//
// N Z C I D V
// + + + - - +
//
OP_FUNCTION(ADC)
{
    uint16_t address = addr_mode( cpu, false );
    uint8_t operand = cpu.fetch_byte( address );
    uint16_t result = operand + cpu.regs.A + cpu.regs.C;

    CALC_N_FLAG( result );
    CALC_Z_FLAG( result );
    CALC_C_FLAG( result );
    CALC_V_FLAG( cpu.regs.A, operand, result );
    cpu.regs.A = result;
}

/////////////////////////////////////////////////////////
// AND - AND Memory with Accumulator
// A AND M -> A
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(AND)
{
    uint16_t address = addr_mode( cpu, false );
    uint8_t operand = cpu.fetch_byte( address );
    uint8_t result = cpu.regs.A & operand;

    CALC_N_FLAG( result );
    CALC_Z_FLAG( result );
    cpu.regs.A = result;
}

/////////////////////////////////////////////////////////
// ASL - Shift Left One Bit (Memory or Accumulator)
// C <- [76543210] <- 0
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(ASL)
{
    uint16_t address = addr_mode( cpu, false );
    uint8_t* operand = cpu.fetch_byte_ref( address );
    uint16_t data = (uint16_t) *operand << 1;
    
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    CALC_C_FLAG( data );

    if ( addr_mode != addr_mode_accumulator )
    {
        cpu.tick_clock();
    }
    cpu.write_byte( data, operand );
}






///////// ------------------------ NOT SORTED ------------------------

/////////////////////////////////////////////////////////
// JMP - Jump to New Location
// (PC+1) -> PCL
// (PC+2) -> PCH
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(JMP)
{
    uint16_t address = addr_mode( cpu, false );
    cpu.regs.PC = address;
}

/////////////////////////////////////////////////////////
// LDX - Load Index X with Memory
// M -> X
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(LDX)
{
    uint16_t address = addr_mode( cpu, false );
    uint8_t operand = cpu.fetch_byte( address );
    cpu.regs.X = operand;
    CALC_Z_FLAG( operand );
    CALC_N_FLAG( operand );
}

/////////////////////////////////////////////////////////
// STX - Store Index X in Memory
// X -> M
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(STX)
{
    uint16_t address = addr_mode( cpu, true );
    cpu.write_byte( cpu.regs.X, address );
}

/////////////////////////////////////////////////////////
// JSR - Jump to New Location Saving Return Address
// push (PC+2)
// (PC+1) -> PCL
// (PC+2) -> PCH
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(JSR)
{
    cpu.push_short_to_stack( cpu.regs.PC + 1 );
    uint16_t address = addr_mode( cpu, false );
    cpu.tick_clock(); // One extra cycle when buffering data
    cpu.regs.PC = address;
}

/////////////////////////////////////////////////////////
// NOP
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(NOP)
{
    addr_mode( cpu, false );
}

/////////////////////////////////////////////////////////
// LDA - Load Accumulator with Memory
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(LDA)
{
    uint16_t address = addr_mode( cpu, false );
    uint8_t operand = cpu.fetch_byte( address );
    cpu.regs.A = operand;
    CALC_Z_FLAG( operand );
    CALC_N_FLAG( operand );
}

/////////////////////////////////////////////////////////
// SEC - Set Carry Flag
// 1 -> C
//
// N Z C I D V
// - - 1 - - -
//
OP_FUNCTION(SEC)
{
    addr_mode( cpu, false );
    cpu.regs.C = 1;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// BCS - Branch on Carry Set
// branch on C = 1
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BCS)
{
    uint16_t old_pc    = cpu.regs.PC;
    uint16_t address   = addr_mode( cpu, false );
    branch( cpu, cpu.regs.C, 1, old_pc, address );
}

/////////////////////////////////////////////////////////
// CLC - Clear Carry Flag
// 0 -> C
//
// N Z C I D V
// - - 0 - - -
//
OP_FUNCTION(CLC)
{
    addr_mode( cpu, false );
    cpu.regs.C = 0;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// BCC - Branch on Carry Clear
// branch on C = 0
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BCC)
{
    uint16_t old_pc    = cpu.regs.PC;
    uint16_t address   = addr_mode( cpu, false );
    branch( cpu, cpu.regs.C, 0, old_pc, address );
}

/////////////////////////////////////////////////////////
// BEQ - Branch on Result Zero
// branch on Z = 1
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BEQ)
{
    uint16_t old_pc    = cpu.regs.PC;
    uint16_t address   = addr_mode( cpu, false );
    branch( cpu, cpu.regs.Z, 1, old_pc, address );
}

/////////////////////////////////////////////////////////
// BNE - Branch on Result not Zero
// branch on Z = 0
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BNE)
{
    uint16_t old_pc    = cpu.regs.PC;
    uint16_t address   = addr_mode( cpu, false );
    branch( cpu, cpu.regs.Z, 0, old_pc, address );
}

/////////////////////////////////////////////////////////
// STA - Store Accumulator in Memory
// A -> M
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(STA)
{
    uint16_t address = addr_mode( cpu, true );
    cpu.write_byte( cpu.regs.A, address );
}

/////////////////////////////////////////////////////////
// BIT - Test Bits in Memory with Accumulator
// bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
// the zero-flag is set to the result of operand AND accumulator.
//
// N Z C I D V
// M7+ - - - M6
//
OP_FUNCTION(BIT)
{
    uint16_t address = addr_mode( cpu, false );
    uint8_t operand = cpu.fetch_byte( address );
    cpu.regs.N = (operand >> 7) & 0x1;
    cpu.regs.V = (operand >> 6) & 0x1;
    cpu.regs.Z = CALC_Z_FLAG( operand & cpu.regs.A );
}

/////////////////////////////////////////////////////////
// BVC - Branch on Overflow Clear
// branch on V = 1
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BVC)
{
    uint16_t old_pc    = cpu.regs.PC;
    uint16_t address   = addr_mode( cpu, false );
    branch( cpu, cpu.regs.V, 0, old_pc, address );
}

/////////////////////////////////////////////////////////
// BVS - Branch on Overflow Set
// branch on V = 1
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BVS)
{
    uint16_t old_pc    = cpu.regs.PC;
    uint16_t address   = addr_mode( cpu, false );
    branch( cpu, cpu.regs.V, 1, old_pc, address );
}

/////////////////////////////////////////////////////////
// BPL - Branch on Result Plus
// branch on N = 0
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BPL)
{
    uint16_t old_pc    = cpu.regs.PC;
    uint16_t address   = addr_mode( cpu, false );
    branch( cpu, cpu.regs.N, 0, old_pc, address );
}

/////////////////////////////////////////////////////////
// RTS - Return from Subroutine
// pull PC, PC+1 -> PC
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(RTS)
{
    addr_mode( cpu, false );
    uint16_t address = cpu.pull_short_from_stack();
    cpu.regs.PC = address + 1;
    cpu.tick_clock(); // One extra cycle to post-increment PC
}

/////////////////////////////////////////////////////////
// SEI - Set Interrupt Disable Status
// 1 -> I
//
// N Z C I D V
// - - - 1 - -
//
OP_FUNCTION(SEI)
{
    addr_mode( cpu, false );
    cpu.regs.I = 1;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// SED - Set Decimal Flag
// 1 -> D
//
// N Z C I D V
// - - - - 1 -
//
OP_FUNCTION(SED)
{
    addr_mode( cpu, false );
    cpu.regs.D = 1;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// PHP - Push Processor Status on Stack
// The status register will be pushed with the break
// flag and bit 5 set to 1.
//
// push SR
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(PHP)
{
    addr_mode( cpu, false );
    uint8_t status = cpu.regs.SR | 0x30;
    cpu.tick_clock();
    cpu.push_byte_to_stack( status );
}

/////////////////////////////////////////////////////////
// PLA - Pull Accumulator from Stack
// pull A
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(PLA)
{
    addr_mode( cpu, true );
    uint8_t data = cpu.pull_byte_from_stack();
    cpu.tick_clock(); // Read-modify-write extra cycle
    cpu.regs.A = data;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
}

/////////////////////////////////////////////////////////
// CMP - Compare Memory with Accumulator
// A - M
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(CMP)
{
    uint16_t address = addr_mode( cpu, false );
    uint8_t operand = cpu.fetch_byte( address );
    uint8_t data = operand - cpu.regs.A;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    cpu.regs.C = (cpu.regs.A >= operand) ? 1 : 0;
}

} // nes