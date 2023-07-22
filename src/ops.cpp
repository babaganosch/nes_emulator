#include "nes_ops.hpp"
#include "nes.hpp"

#define CALC_Z_FLAG(VALUE) cpu.regs.Z = (((VALUE & 0xFF) == 0x00) ? 1 : 0 )
#define CALC_N_FLAG(VALUE) cpu.regs.N = (((VALUE & 0x80) > 0x00) ? 1 : 0 )
#define CALC_C_FLAG(VALUE) cpu.regs.C = (( VALUE > 0xFF ) ? 1 : 0 )
#define CALC_V_FLAG(M, N, RESULT) cpu.regs.V = ((((M ^ RESULT) & (N ^ RESULT) & 0x80) != 0) ? 1 : 0 )

#define UINT16(LO, HI) (((uint16_t) HI << 8) | LO)

namespace nes
{

op_code_t op_codes[256] = {
    CPU_OP(NIP, implied),                //   0     $ 00
    CPU_OP(NIP, implied),                //   1     $ 01
    CPU_OP(NIP, implied),                //   2     $ 02
    CPU_OP(NIP, implied),                //   3     $ 03
    CPU_OP(NIP, implied),                //   4     $ 04
    CPU_OP(NIP, implied),                //   5     $ 05
    CPU_OP(ASL, zero_page),              //   6     $ 06
    CPU_OP(NIP, implied),                //   7     $ 07
    CPU_OP(NIP, implied),                //   8     $ 08
    CPU_OP(NIP, implied),                //   9     $ 09
    CPU_OP(ASL, accumulator),            //  10     $ 0A
    CPU_OP(NIP, implied),                //  11     $ 0B
    CPU_OP(NIP, implied),                //  12     $ 0C
    CPU_OP(NIP, implied),                //  13     $ 0D
    CPU_OP(ASL, absolute),               //  14     $ 0E
    CPU_OP(NIP, implied),                //  15     $ 0F
    CPU_OP(NIP, implied),                //  16     $ 10
    CPU_OP(NIP, implied),                //  17     $ 11
    CPU_OP(NIP, implied),                //  18     $ 12
    CPU_OP(NIP, implied),                //  19     $ 13
    CPU_OP(NIP, implied),                //  20     $ 14
    CPU_OP(NIP, implied),                //  21     $ 15
    CPU_OP(ASL, index_zp_x),             //  22     $ 16
    CPU_OP(NIP, implied),                //  23     $ 17
    CPU_OP(CLC, implied),                //  24     $ 18
    CPU_OP(NIP, implied),                //  25     $ 19
    CPU_OP(NIP, implied),                //  26     $ 1A
    CPU_OP(NIP, implied),                //  27     $ 1B
    CPU_OP(NIP, implied),                //  28     $ 1C
    CPU_OP(NIP, implied),                //  29     $ 1D
    CPU_OP(ASL, index_x),                //  30     $ 1E
    CPU_OP(NIP, implied),                //  31     $ 1F
    CPU_OP(JSR, absolute),               //  32     $ 20
    CPU_OP(AND, pre_index_indirect_x),   //  33     $ 21
    CPU_OP(NIP, implied),                //  34     $ 22
    CPU_OP(NIP, implied),                //  35     $ 23
    CPU_OP(NIP, implied),                //  36     $ 24
    CPU_OP(AND, zero_page),              //  37     $ 25
    CPU_OP(NIP, implied),                //  38     $ 26
    CPU_OP(NIP, implied),                //  39     $ 27
    CPU_OP(NIP, implied),                //  40     $ 28
    CPU_OP(AND, immediate),              //  41     $ 29
    CPU_OP(NIP, implied),                //  42     $ 2A
    CPU_OP(NIP, implied),                //  43     $ 2B
    CPU_OP(NIP, implied),                //  44     $ 2C
    CPU_OP(AND, absolute),               //  45     $ 2D
    CPU_OP(NIP, implied),                //  46     $ 2E
    CPU_OP(NIP, implied),                //  47     $ 2F
    CPU_OP(NIP, implied),                //  48     $ 30
    CPU_OP(AND, post_index_indirect_y),  //  49     $ 31
    CPU_OP(NIP, implied),                //  50     $ 32
    CPU_OP(NIP, implied),                //  51     $ 33
    CPU_OP(NIP, implied),                //  52     $ 34
    CPU_OP(AND, index_zp_x),             //  53     $ 35
    CPU_OP(NIP, implied),                //  54     $ 36
    CPU_OP(NIP, implied),                //  55     $ 37
    CPU_OP(SEC, implied),                //  56     $ 38
    CPU_OP(AND, index_y),                //  57     $ 39
    CPU_OP(NIP, implied),                //  58     $ 3A
    CPU_OP(NIP, implied),                //  59     $ 3B
    CPU_OP(NIP, implied),                //  60     $ 3C
    CPU_OP(AND, index_x),                //  61     $ 3D
    CPU_OP(NIP, implied),                //  62     $ 3E
    CPU_OP(NIP, implied),                //  63     $ 3F
    CPU_OP(NIP, implied),                //  64     $ 40
    CPU_OP(NIP, implied),                //  65     $ 41
    CPU_OP(NIP, implied),                //  66     $ 42
    CPU_OP(NIP, implied),                //  67     $ 43
    CPU_OP(NIP, implied),                //  68     $ 44
    CPU_OP(NIP, implied),                //  69     $ 45
    CPU_OP(NIP, implied),                //  70     $ 46
    CPU_OP(NIP, implied),                //  71     $ 47
    CPU_OP(NIP, implied),                //  72     $ 48
    CPU_OP(NIP, implied),                //  73     $ 49
    CPU_OP(NIP, implied),                //  74     $ 4A
    CPU_OP(NIP, implied),                //  75     $ 4B
    CPU_OP(JMP, absolute),               //  76     $ 4C
    CPU_OP(NIP, implied),                //  77     $ 4D
    CPU_OP(NIP, implied),                //  78     $ 4E
    CPU_OP(NIP, implied),                //  79     $ 4F
    CPU_OP(NIP, implied),                //  80     $ 50
    CPU_OP(NIP, implied),                //  81     $ 51
    CPU_OP(NIP, implied),                //  82     $ 52
    CPU_OP(NIP, implied),                //  83     $ 53
    CPU_OP(NIP, implied),                //  84     $ 54
    CPU_OP(NIP, implied),                //  85     $ 55
    CPU_OP(NIP, implied),                //  86     $ 56
    CPU_OP(NIP, implied),                //  87     $ 57
    CPU_OP(NIP, implied),                //  88     $ 58
    CPU_OP(NIP, implied),                //  89     $ 59
    CPU_OP(NIP, implied),                //  90     $ 5A
    CPU_OP(NIP, implied),                //  91     $ 5B
    CPU_OP(NIP, implied),                //  92     $ 5C
    CPU_OP(NIP, implied),                //  93     $ 5D
    CPU_OP(NIP, implied),                //  94     $ 5E
    CPU_OP(NIP, implied),                //  95     $ 5F
    CPU_OP(NIP, implied),                //  96     $ 60
    CPU_OP(ADC, pre_index_indirect_x),   //  97     $ 61
    CPU_OP(NIP, implied),                //  98     $ 62
    CPU_OP(NIP, implied),                //  99     $ 63
    CPU_OP(NIP, implied),                // 100     $ 64
    CPU_OP(ADC, zero_page),              // 101     $ 65
    CPU_OP(NIP, implied),                // 102     $ 66
    CPU_OP(NIP, implied),                // 103     $ 67
    CPU_OP(NIP, implied),                // 104     $ 68
    CPU_OP(ADC, immediate),              // 105     $ 69
    CPU_OP(NIP, implied),                // 106     $ 6A
    CPU_OP(NIP, implied),                // 107     $ 6B
    CPU_OP(JMP, indirect),               // 108     $ 6C
    CPU_OP(ADC, absolute),               // 109     $ 6D
    CPU_OP(NIP, implied),                // 110     $ 6E
    CPU_OP(NIP, implied),                // 111     $ 6F
    CPU_OP(NIP, implied),                // 112     $ 70
    CPU_OP(ADC, post_index_indirect_y),  // 113     $ 71
    CPU_OP(NIP, implied),                // 114     $ 72
    CPU_OP(NIP, implied),                // 115     $ 73
    CPU_OP(NIP, implied),                // 116     $ 74
    CPU_OP(ADC, index_zp_x),             // 117     $ 75
    CPU_OP(NIP, implied),                // 118     $ 76
    CPU_OP(NIP, implied),                // 119     $ 77
    CPU_OP(NIP, implied),                // 120     $ 78
    CPU_OP(ADC, index_y),                // 121     $ 79
    CPU_OP(NIP, implied),                // 122     $ 7A
    CPU_OP(NIP, implied),                // 123     $ 7B
    CPU_OP(NIP, implied),                // 124     $ 7C
    CPU_OP(ADC, index_x),                // 125     $ 7D
    CPU_OP(NIP, implied),                // 126     $ 7E
    CPU_OP(NIP, implied),                // 127     $ 7F
    CPU_OP(NIP, implied),                // 128     $ 80
    CPU_OP(NIP, implied),                // 129     $ 81
    CPU_OP(NIP, implied),                // 130     $ 82
    CPU_OP(NIP, implied),                // 131     $ 83
    CPU_OP(NIP, implied),                // 132     $ 84
    CPU_OP(NIP, implied),                // 133     $ 85
    CPU_OP(STX, zero_page),              // 134     $ 86
    CPU_OP(NIP, implied),                // 135     $ 87
    CPU_OP(NIP, implied),                // 136     $ 88
    CPU_OP(NIP, implied),                // 137     $ 89
    CPU_OP(NIP, implied),                // 138     $ 8A
    CPU_OP(NIP, implied),                // 139     $ 8B
    CPU_OP(NIP, implied),                // 140     $ 8C
    CPU_OP(NIP, implied),                // 141     $ 8D
    CPU_OP(STX, absolute),               // 142     $ 8E
    CPU_OP(NIP, implied),                // 143     $ 8F
    CPU_OP(BCC, relative),               // 144     $ 90
    CPU_OP(NIP, implied),                // 145     $ 91
    CPU_OP(NIP, implied),                // 146     $ 92
    CPU_OP(NIP, implied),                // 147     $ 93
    CPU_OP(NIP, implied),                // 148     $ 94
    CPU_OP(NIP, implied),                // 149     $ 95
    CPU_OP(STX, index_zp_y),             // 150     $ 96
    CPU_OP(NIP, implied),                // 151     $ 97
    CPU_OP(NIP, implied),                // 152     $ 98
    CPU_OP(NIP, implied),                // 153     $ 99
    CPU_OP(NIP, implied),                // 154     $ 9A
    CPU_OP(NIP, implied),                // 155     $ 9B
    CPU_OP(NIP, implied),                // 156     $ 9C
    CPU_OP(NIP, implied),                // 157     $ 9D
    CPU_OP(NIP, implied),                // 158     $ 9E
    CPU_OP(NIP, implied),                // 159     $ 9F
    CPU_OP(NIP, implied),                // 160     $ A0
    CPU_OP(NIP, implied),                // 161     $ A1
    CPU_OP(LDX, immediate),              // 162     $ A2
    CPU_OP(NIP, implied),                // 163     $ A3
    CPU_OP(NIP, implied),                // 164     $ A4
    CPU_OP(LDA, zero_page),              // 165     $ A5
    CPU_OP(LDX, zero_page),              // 166     $ A6
    CPU_OP(NIP, implied),                // 167     $ A7
    CPU_OP(NIP, implied),                // 168     $ A8
    CPU_OP(LDA, immediate),              // 169     $ A9
    CPU_OP(NIP, implied),                // 170     $ AA
    CPU_OP(NIP, implied),                // 171     $ AB
    CPU_OP(NIP, implied),                // 172     $ AC
    CPU_OP(LDA, absolute),               // 173     $ AD
    CPU_OP(LDX, absolute),               // 174     $ AE
    CPU_OP(NIP, implied),                // 175     $ AF
    CPU_OP(BCS, relative),               // 176     $ B0
    CPU_OP(LDA, post_index_indirect_y),  // 177     $ B1
    CPU_OP(NIP, implied),                // 178     $ B2
    CPU_OP(NIP, implied),                // 179     $ B3
    CPU_OP(NIP, implied),                // 180     $ B4
    CPU_OP(LDA, index_zp_x),             // 181     $ B5
    CPU_OP(LDX, index_zp_x),             // 182     $ B6
    CPU_OP(NIP, implied),                // 183     $ B7
    CPU_OP(NIP, implied),                // 184     $ B8
    CPU_OP(LDA, index_y),                // 185     $ B9
    CPU_OP(NIP, implied),                // 186     $ BA
    CPU_OP(NIP, implied),                // 187     $ BB
    CPU_OP(NIP, implied),                // 188     $ BC
    CPU_OP(LDA, index_x),                // 189     $ BD
    CPU_OP(LDX, index_y),                // 190     $ BE
    CPU_OP(NIP, implied),                // 191     $ BF
    CPU_OP(NIP, implied),                // 192     $ C0
    CPU_OP(NIP, implied),                // 193     $ C1
    CPU_OP(NIP, implied),                // 194     $ C2
    CPU_OP(NIP, implied),                // 195     $ C3
    CPU_OP(NIP, implied),                // 196     $ C4
    CPU_OP(NIP, implied),                // 197     $ C5
    CPU_OP(NIP, implied),                // 198     $ C6
    CPU_OP(NIP, implied),                // 199     $ C7
    CPU_OP(NIP, implied),                // 200     $ C8
    CPU_OP(NIP, implied),                // 201     $ C9
    CPU_OP(NIP, implied),                // 202     $ CA
    CPU_OP(NIP, implied),                // 203     $ CB
    CPU_OP(NIP, implied),                // 204     $ CC
    CPU_OP(NIP, implied),                // 205     $ CD
    CPU_OP(NIP, implied),                // 206     $ CE
    CPU_OP(NIP, implied),                // 207     $ CF
    CPU_OP(BNE, relative),               // 208     $ D0
    CPU_OP(NIP, implied),                // 209     $ D1
    CPU_OP(NIP, implied),                // 210     $ D2
    CPU_OP(NIP, implied),                // 211     $ D3
    CPU_OP(NIP, implied),                // 212     $ D4
    CPU_OP(NIP, implied),                // 213     $ D5
    CPU_OP(NIP, implied),                // 214     $ D6
    CPU_OP(NIP, implied),                // 215     $ D7
    CPU_OP(NIP, implied),                // 216     $ D8
    CPU_OP(NIP, implied),                // 217     $ D9
    CPU_OP(NIP, implied),                // 218     $ DA
    CPU_OP(NIP, implied),                // 219     $ DB
    CPU_OP(NIP, implied),                // 220     $ DC
    CPU_OP(NIP, implied),                // 221     $ DD
    CPU_OP(NIP, implied),                // 222     $ DE
    CPU_OP(NIP, implied),                // 223     $ DF
    CPU_OP(NIP, implied),                // 224     $ E0
    CPU_OP(NIP, implied),                // 225     $ E1
    CPU_OP(NIP, implied),                // 226     $ E2
    CPU_OP(NIP, implied),                // 227     $ E3
    CPU_OP(NIP, implied),                // 228     $ E4
    CPU_OP(NIP, implied),                // 229     $ E5
    CPU_OP(NIP, implied),                // 230     $ E6
    CPU_OP(NIP, implied),                // 231     $ E7
    CPU_OP(NIP, implied),                // 232     $ E8
    CPU_OP(NIP, implied),                // 233     $ E9
    CPU_OP(NOP, implied),                // 234     $ EA
    CPU_OP(NIP, implied),                // 235     $ EB
    CPU_OP(NIP, implied),                // 236     $ EC
    CPU_OP(NIP, implied),                // 237     $ ED
    CPU_OP(NIP, implied),                // 238     $ EE
    CPU_OP(NIP, implied),                // 239     $ EF
    CPU_OP(BEQ, relative),               // 240     $ F0
    CPU_OP(NIP, implied),                // 241     $ F1
    CPU_OP(NIP, implied),                // 242     $ F2
    CPU_OP(NIP, implied),                // 243     $ F3
    CPU_OP(NIP, implied),                // 244     $ F4
    CPU_OP(NIP, implied),                // 245     $ F5
    CPU_OP(NIP, implied),                // 246     $ F6
    CPU_OP(NIP, implied),                // 247     $ F7
    CPU_OP(NIP, implied),                // 248     $ F8
    CPU_OP(NIP, implied),                // 249     $ F9
    CPU_OP(NIP, implied),                // 250     $ FA
    CPU_OP(NIP, implied),                // 251     $ FB
    CPU_OP(NIP, implied),                // 252     $ FC
    CPU_OP(NIP, implied),                // 253     $ FD
    CPU_OP(NIP, implied),                // 254     $ FE
    CPU_OP(NIP, implied)                 // 255     $ FF
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
OP_FUNCTION(NIP)
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
    uint16_t address = addr_mode(cpu, false);
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
    uint16_t address = addr_mode(cpu, false);
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
    /*
    uint16_t address = addr_mode(cpu, false);
    uint8_t* operand = cpu.fetch_byte_ref( address );
    uint16_t data = (uint16_t) *operand << 1;
    
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    CALC_C_FLAG( data );

    if ( addr_mode != addr_mode_accumulator )
    {
        cpu.tick_clock();
    }
    cpu.write_byte( data, operand );*/
}


///////// ------------------------

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
    uint16_t address = addr_mode(cpu, false);
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
    uint16_t address = addr_mode(cpu, false);
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
    uint16_t address = addr_mode(cpu, true);
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
    uint16_t address = addr_mode(cpu, false);
    cpu.tick_clock(); // One extra cycle when buffering data
    cpu.push_short_to_stack( cpu.regs.PC + 2 );
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
    addr_mode(cpu, false);
}

/////////////////////////////////////////////////////////
// LDA - Load Accumulator with Memory
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(LDA)
{
    uint16_t address = addr_mode(cpu, false);
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
    addr_mode(cpu, false);
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
    uint16_t old_pc = cpu.regs.PC;
    uint16_t address = addr_mode(cpu, false);
    if (cpu.regs.C == 1)
    { // Branch occurs
        cpu.tick_clock();
        if ( (old_pc & 0xFF00) != (address & 0xFF00) )
        { // Branch caused page change
            cpu.tick_clock();
        }
        cpu.regs.PC = address;
    }
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
    addr_mode(cpu, false);
    cpu.regs.C = 0;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// BCS - Branch on Carry Set
// branch on C = 0
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BCC)
{
    uint16_t old_pc = cpu.regs.PC;
    uint16_t address = addr_mode(cpu, false);
    if (cpu.regs.C == 0)
    { // Branch occurs
        cpu.tick_clock();
        if ( (old_pc & 0xFF00) != (address & 0xFF00) )
        { // Branch caused page change
            cpu.tick_clock();
        }
        cpu.regs.PC = address;
    }
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
    uint16_t old_pc = cpu.regs.PC;
    uint16_t address = addr_mode(cpu, false);
    if (cpu.regs.Z == 1)
    { // Branch occurs
        cpu.tick_clock();
        if ( (old_pc & 0xFF00) != (address & 0xFF00) )
        { // Branch caused page change
            cpu.tick_clock();
        }
        cpu.regs.PC = address;
    }
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
    uint16_t old_pc = cpu.regs.PC;
    uint16_t address = addr_mode(cpu, false);
    if (cpu.regs.Z == 0)
    { // Branch occurs
        cpu.tick_clock();
        if ( (old_pc & 0xFF00) != (address & 0xFF00) )
        { // Branch caused page change
            cpu.tick_clock();
        }
        cpu.regs.PC = address;
    }
}


} // nes