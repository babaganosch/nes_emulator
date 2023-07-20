#include "nes_ops.hpp"
#include "nes.hpp"

#define CALC_Z_FLAG(VALUE) cpu.regs.Z = (((VALUE & 0xFF) == 0x00) ? 1 : 0 )
#define CALC_N_FLAG(VALUE) cpu.regs.N = (((VALUE & 0x80) > 0x00) ? 1 : 0 )
#define CALC_C_FLAG(VALUE) cpu.regs.C = (( VALUE > 0xFF ) ? 1 : 0 )
#define CALC_V_FLAG(M, N, RESULT) cpu.regs.V = ((((M ^ RESULT) & (N ^ RESULT) & 0x80) != 0) ? 1 : 0 )

namespace nes
{

op_code_t op_codes[256] = {
    CPU_OP(implied, UNIMPLEMENTED),      //   0     $ 00
    CPU_OP(implied, UNIMPLEMENTED),      //   1     $ 01
    CPU_OP(implied, UNIMPLEMENTED),      //   2     $ 02
    CPU_OP(implied, UNIMPLEMENTED),      //   3     $ 03
    CPU_OP(implied, UNIMPLEMENTED),      //   4     $ 04
    CPU_OP(implied, UNIMPLEMENTED),      //   5     $ 05
    CPU_OP(zero_page, ASL),              //   6     $ 06
    CPU_OP(implied, UNIMPLEMENTED),      //   7     $ 07
    CPU_OP(implied, UNIMPLEMENTED),      //   8     $ 08
    CPU_OP(implied, UNIMPLEMENTED),      //   9     $ 09
    CPU_OP(accumulator, ASL),            //  10     $ 0A
    CPU_OP(implied, UNIMPLEMENTED),      //  11     $ 0B
    CPU_OP(implied, UNIMPLEMENTED),      //  12     $ 0C
    CPU_OP(implied, UNIMPLEMENTED),      //  13     $ 0D
    CPU_OP(absolute, ASL),               //  14     $ 0E
    CPU_OP(implied, UNIMPLEMENTED),      //  15     $ 0F
    CPU_OP(implied, UNIMPLEMENTED),      //  16     $ 10
    CPU_OP(implied, UNIMPLEMENTED),      //  17     $ 11
    CPU_OP(implied, UNIMPLEMENTED),      //  18     $ 12
    CPU_OP(implied, UNIMPLEMENTED),      //  19     $ 13
    CPU_OP(implied, UNIMPLEMENTED),      //  20     $ 14
    CPU_OP(implied, UNIMPLEMENTED),      //  21     $ 15
    CPU_OP(index_zp_x, ASL),             //  22     $ 16
    CPU_OP(implied, UNIMPLEMENTED),      //  23     $ 17
    CPU_OP(implied, UNIMPLEMENTED),      //  24     $ 18
    CPU_OP(implied, UNIMPLEMENTED),      //  25     $ 19
    CPU_OP(implied, UNIMPLEMENTED),      //  26     $ 1A
    CPU_OP(implied, UNIMPLEMENTED),      //  27     $ 1B
    CPU_OP(implied, UNIMPLEMENTED),      //  28     $ 1C
    CPU_OP(implied, UNIMPLEMENTED),      //  29     $ 1D
    CPU_OP(index_x, ASL),                //  30     $ 1E
    CPU_OP(implied, UNIMPLEMENTED),      //  31     $ 1F
    CPU_OP(implied, UNIMPLEMENTED),      //  32     $ 20
    CPU_OP(pre_index_indirect_x, AND),   //  33     $ 21
    CPU_OP(implied, UNIMPLEMENTED),      //  34     $ 22
    CPU_OP(implied, UNIMPLEMENTED),      //  35     $ 23
    CPU_OP(implied, UNIMPLEMENTED),      //  36     $ 24
    CPU_OP(zero_page, AND),              //  37     $ 25
    CPU_OP(implied, UNIMPLEMENTED),      //  38     $ 26
    CPU_OP(implied, UNIMPLEMENTED),      //  39     $ 27
    CPU_OP(implied, UNIMPLEMENTED),      //  40     $ 28
    CPU_OP(immediate, AND),              //  41     $ 29
    CPU_OP(implied, UNIMPLEMENTED),      //  42     $ 2A
    CPU_OP(implied, UNIMPLEMENTED),      //  43     $ 2B
    CPU_OP(implied, UNIMPLEMENTED),      //  44     $ 2C
    CPU_OP(absolute, AND),               //  45     $ 2D
    CPU_OP(implied, UNIMPLEMENTED),      //  46     $ 2E
    CPU_OP(implied, UNIMPLEMENTED),      //  47     $ 2F
    CPU_OP(implied, UNIMPLEMENTED),      //  48     $ 30
    CPU_OP(post_index_indirect_y, AND),  //  49     $ 31
    CPU_OP(implied, UNIMPLEMENTED),      //  50     $ 32
    CPU_OP(implied, UNIMPLEMENTED),      //  51     $ 33
    CPU_OP(implied, UNIMPLEMENTED),      //  52     $ 34
    CPU_OP(index_zp_x, AND),             //  53     $ 35
    CPU_OP(implied, UNIMPLEMENTED),      //  54     $ 36
    CPU_OP(implied, UNIMPLEMENTED),      //  55     $ 37
    CPU_OP(implied, UNIMPLEMENTED),      //  56     $ 38
    CPU_OP(index_y, AND),                //  57     $ 39
    CPU_OP(implied, UNIMPLEMENTED),      //  58     $ 3A
    CPU_OP(implied, UNIMPLEMENTED),      //  59     $ 3B
    CPU_OP(implied, UNIMPLEMENTED),      //  60     $ 3C
    CPU_OP(index_x, AND),                //  61     $ 3D
    CPU_OP(implied, UNIMPLEMENTED),      //  62     $ 3E
    CPU_OP(implied, UNIMPLEMENTED),      //  63     $ 3F
    CPU_OP(implied, UNIMPLEMENTED),      //  64     $ 40
    CPU_OP(implied, UNIMPLEMENTED),      //  65     $ 41
    CPU_OP(implied, UNIMPLEMENTED),      //  66     $ 42
    CPU_OP(implied, UNIMPLEMENTED),      //  67     $ 43
    CPU_OP(implied, UNIMPLEMENTED),      //  68     $ 44
    CPU_OP(implied, UNIMPLEMENTED),      //  69     $ 45
    CPU_OP(implied, UNIMPLEMENTED),      //  70     $ 46
    CPU_OP(implied, UNIMPLEMENTED),      //  71     $ 47
    CPU_OP(implied, UNIMPLEMENTED),      //  72     $ 48
    CPU_OP(implied, UNIMPLEMENTED),      //  73     $ 49
    CPU_OP(implied, UNIMPLEMENTED),      //  74     $ 4A
    CPU_OP(implied, UNIMPLEMENTED),      //  75     $ 4B
    CPU_OP(implied, UNIMPLEMENTED),      //  76     $ 4C
    CPU_OP(implied, UNIMPLEMENTED),      //  77     $ 4D
    CPU_OP(implied, UNIMPLEMENTED),      //  78     $ 4E
    CPU_OP(implied, UNIMPLEMENTED),      //  79     $ 4F
    CPU_OP(implied, UNIMPLEMENTED),      //  80     $ 50
    CPU_OP(implied, UNIMPLEMENTED),      //  81     $ 51
    CPU_OP(implied, UNIMPLEMENTED),      //  82     $ 52
    CPU_OP(implied, UNIMPLEMENTED),      //  83     $ 53
    CPU_OP(implied, UNIMPLEMENTED),      //  84     $ 54
    CPU_OP(implied, UNIMPLEMENTED),      //  85     $ 55
    CPU_OP(implied, UNIMPLEMENTED),      //  86     $ 56
    CPU_OP(implied, UNIMPLEMENTED),      //  87     $ 57
    CPU_OP(implied, UNIMPLEMENTED),      //  88     $ 58
    CPU_OP(implied, UNIMPLEMENTED),      //  89     $ 59
    CPU_OP(implied, UNIMPLEMENTED),      //  90     $ 5A
    CPU_OP(implied, UNIMPLEMENTED),      //  91     $ 5B
    CPU_OP(implied, UNIMPLEMENTED),      //  92     $ 5C
    CPU_OP(implied, UNIMPLEMENTED),      //  93     $ 5D
    CPU_OP(implied, UNIMPLEMENTED),      //  94     $ 5E
    CPU_OP(implied, UNIMPLEMENTED),      //  95     $ 5F
    CPU_OP(implied, UNIMPLEMENTED),      //  96     $ 60
    CPU_OP(pre_index_indirect_x, ADC),   //  97     $ 61
    CPU_OP(implied, UNIMPLEMENTED),      //  98     $ 62
    CPU_OP(implied, UNIMPLEMENTED),      //  99     $ 63
    CPU_OP(implied, UNIMPLEMENTED),      // 100     $ 64
    CPU_OP(zero_page, ADC),              // 101     $ 65
    CPU_OP(implied, UNIMPLEMENTED),      // 102     $ 66
    CPU_OP(implied, UNIMPLEMENTED),      // 103     $ 67
    CPU_OP(implied, UNIMPLEMENTED),      // 104     $ 68
    CPU_OP(immediate, ADC),              // 105     $ 69
    CPU_OP(implied, UNIMPLEMENTED),      // 106     $ 6A
    CPU_OP(implied, UNIMPLEMENTED),      // 107     $ 6B
    CPU_OP(implied, UNIMPLEMENTED),      // 108     $ 6C
    CPU_OP(absolute, ADC),               // 109     $ 6D
    CPU_OP(implied, UNIMPLEMENTED),      // 110     $ 6E
    CPU_OP(implied, UNIMPLEMENTED),      // 111     $ 6F
    CPU_OP(implied, UNIMPLEMENTED),      // 112     $ 70
    CPU_OP(post_index_indirect_y, ADC),  // 113     $ 71
    CPU_OP(implied, UNIMPLEMENTED),      // 114     $ 72
    CPU_OP(implied, UNIMPLEMENTED),      // 115     $ 73
    CPU_OP(implied, UNIMPLEMENTED),      // 116     $ 74
    CPU_OP(index_zp_x, ADC),             // 117     $ 75
    CPU_OP(implied, UNIMPLEMENTED),      // 118     $ 76
    CPU_OP(implied, UNIMPLEMENTED),      // 119     $ 77
    CPU_OP(implied, UNIMPLEMENTED),      // 120     $ 78
    CPU_OP(index_y, ADC),                // 121     $ 79
    CPU_OP(implied, UNIMPLEMENTED),      // 122     $ 7A
    CPU_OP(implied, UNIMPLEMENTED),      // 123     $ 7B
    CPU_OP(implied, UNIMPLEMENTED),      // 124     $ 7C
    CPU_OP(index_x, ADC),                // 125     $ 7D
    CPU_OP(implied, UNIMPLEMENTED),      // 126     $ 7E
    CPU_OP(implied, UNIMPLEMENTED),      // 127     $ 7F
    CPU_OP(implied, UNIMPLEMENTED),      // 128     $ 80
    CPU_OP(implied, UNIMPLEMENTED),      // 129     $ 81
    CPU_OP(implied, UNIMPLEMENTED),      // 130     $ 82
    CPU_OP(implied, UNIMPLEMENTED),      // 131     $ 83
    CPU_OP(implied, UNIMPLEMENTED),      // 132     $ 84
    CPU_OP(implied, UNIMPLEMENTED),      // 133     $ 85
    CPU_OP(implied, UNIMPLEMENTED),      // 134     $ 86
    CPU_OP(implied, UNIMPLEMENTED),      // 135     $ 87
    CPU_OP(implied, UNIMPLEMENTED),      // 136     $ 88
    CPU_OP(implied, UNIMPLEMENTED),      // 137     $ 89
    CPU_OP(implied, UNIMPLEMENTED),      // 138     $ 8A
    CPU_OP(implied, UNIMPLEMENTED),      // 139     $ 8B
    CPU_OP(implied, UNIMPLEMENTED),      // 140     $ 8C
    CPU_OP(implied, UNIMPLEMENTED),      // 141     $ 8D
    CPU_OP(implied, UNIMPLEMENTED),      // 142     $ 8E
    CPU_OP(implied, UNIMPLEMENTED),      // 143     $ 8F
    CPU_OP(implied, UNIMPLEMENTED),      // 144     $ 90
    CPU_OP(implied, UNIMPLEMENTED),      // 145     $ 91
    CPU_OP(implied, UNIMPLEMENTED),      // 146     $ 92
    CPU_OP(implied, UNIMPLEMENTED),      // 147     $ 93
    CPU_OP(implied, UNIMPLEMENTED),      // 148     $ 94
    CPU_OP(implied, UNIMPLEMENTED),      // 149     $ 95
    CPU_OP(implied, UNIMPLEMENTED),      // 150     $ 96
    CPU_OP(implied, UNIMPLEMENTED),      // 151     $ 97
    CPU_OP(implied, UNIMPLEMENTED),      // 152     $ 98
    CPU_OP(implied, UNIMPLEMENTED),      // 153     $ 99
    CPU_OP(implied, UNIMPLEMENTED),      // 154     $ 9A
    CPU_OP(implied, UNIMPLEMENTED),      // 155     $ 9B
    CPU_OP(implied, UNIMPLEMENTED),      // 156     $ 9C
    CPU_OP(implied, UNIMPLEMENTED),      // 157     $ 9D
    CPU_OP(implied, UNIMPLEMENTED),      // 158     $ 9E
    CPU_OP(implied, UNIMPLEMENTED),      // 159     $ 9F
    CPU_OP(implied, UNIMPLEMENTED),      // 160     $ A0
    CPU_OP(pre_index_indirect_x, LDA),   // 161     $ A1
    CPU_OP(implied, UNIMPLEMENTED),      // 162     $ A2
    CPU_OP(implied, UNIMPLEMENTED),      // 163     $ A3
    CPU_OP(implied, UNIMPLEMENTED),      // 164     $ A4
    CPU_OP(zero_page, LDA),              // 165     $ A5
    CPU_OP(implied, UNIMPLEMENTED),      // 166     $ A6
    CPU_OP(implied, UNIMPLEMENTED),      // 167     $ A7
    CPU_OP(implied, UNIMPLEMENTED),      // 168     $ A8
    CPU_OP(immediate, LDA),              // 169     $ A9
    CPU_OP(implied, UNIMPLEMENTED),      // 170     $ AA
    CPU_OP(implied, UNIMPLEMENTED),      // 171     $ AB
    CPU_OP(implied, UNIMPLEMENTED),      // 172     $ AC
    CPU_OP(absolute, LDA),               // 173     $ AD
    CPU_OP(implied, UNIMPLEMENTED),      // 174     $ AE
    CPU_OP(implied, UNIMPLEMENTED),      // 175     $ AF
    CPU_OP(implied, UNIMPLEMENTED),      // 176     $ B0
    CPU_OP(post_index_indirect_y, LDA),  // 177     $ B1
    CPU_OP(implied, UNIMPLEMENTED),      // 178     $ B2
    CPU_OP(implied, UNIMPLEMENTED),      // 179     $ B3
    CPU_OP(implied, UNIMPLEMENTED),      // 180     $ B4
    CPU_OP(index_zp_x, LDA),             // 181     $ B5
    CPU_OP(implied, UNIMPLEMENTED),      // 182     $ B6
    CPU_OP(implied, UNIMPLEMENTED),      // 183     $ B7
    CPU_OP(implied, UNIMPLEMENTED),      // 184     $ B8
    CPU_OP(index_y, LDA),                // 185     $ B9
    CPU_OP(implied, UNIMPLEMENTED),      // 186     $ BA
    CPU_OP(implied, UNIMPLEMENTED),      // 187     $ BB
    CPU_OP(implied, UNIMPLEMENTED),      // 188     $ BC
    CPU_OP(index_x, LDA),                // 189     $ BD
    CPU_OP(implied, UNIMPLEMENTED),      // 190     $ BE
    CPU_OP(implied, UNIMPLEMENTED),      // 191     $ BF
    CPU_OP(implied, UNIMPLEMENTED),      // 192     $ C0
    CPU_OP(implied, UNIMPLEMENTED),      // 193     $ C1
    CPU_OP(implied, UNIMPLEMENTED),      // 194     $ C2
    CPU_OP(implied, UNIMPLEMENTED),      // 195     $ C3
    CPU_OP(implied, UNIMPLEMENTED),      // 196     $ C4
    CPU_OP(implied, UNIMPLEMENTED),      // 197     $ C5
    CPU_OP(implied, UNIMPLEMENTED),      // 198     $ C6
    CPU_OP(implied, UNIMPLEMENTED),      // 199     $ C7
    CPU_OP(implied, UNIMPLEMENTED),      // 200     $ C8
    CPU_OP(implied, UNIMPLEMENTED),      // 201     $ C9
    CPU_OP(implied, UNIMPLEMENTED),      // 202     $ CA
    CPU_OP(implied, UNIMPLEMENTED),      // 203     $ CB
    CPU_OP(implied, UNIMPLEMENTED),      // 204     $ CC
    CPU_OP(implied, UNIMPLEMENTED),      // 205     $ CD
    CPU_OP(implied, UNIMPLEMENTED),      // 206     $ CE
    CPU_OP(implied, UNIMPLEMENTED),      // 207     $ CF
    CPU_OP(implied, UNIMPLEMENTED),      // 208     $ D0
    CPU_OP(implied, UNIMPLEMENTED),      // 209     $ D1
    CPU_OP(implied, UNIMPLEMENTED),      // 210     $ D2
    CPU_OP(implied, UNIMPLEMENTED),      // 211     $ D3
    CPU_OP(implied, UNIMPLEMENTED),      // 212     $ D4
    CPU_OP(implied, UNIMPLEMENTED),      // 213     $ D5
    CPU_OP(implied, UNIMPLEMENTED),      // 214     $ D6
    CPU_OP(implied, UNIMPLEMENTED),      // 215     $ D7
    CPU_OP(implied, UNIMPLEMENTED),      // 216     $ D8
    CPU_OP(implied, UNIMPLEMENTED),      // 217     $ D9
    CPU_OP(implied, UNIMPLEMENTED),      // 218     $ DA
    CPU_OP(implied, UNIMPLEMENTED),      // 219     $ DB
    CPU_OP(implied, UNIMPLEMENTED),      // 220     $ DC
    CPU_OP(implied, UNIMPLEMENTED),      // 221     $ DD
    CPU_OP(implied, UNIMPLEMENTED),      // 222     $ DE
    CPU_OP(implied, UNIMPLEMENTED),      // 223     $ DF
    CPU_OP(implied, UNIMPLEMENTED),      // 224     $ E0
    CPU_OP(implied, UNIMPLEMENTED),      // 225     $ E1
    CPU_OP(implied, UNIMPLEMENTED),      // 226     $ E2
    CPU_OP(implied, UNIMPLEMENTED),      // 227     $ E3
    CPU_OP(implied, UNIMPLEMENTED),      // 228     $ E4
    CPU_OP(implied, UNIMPLEMENTED),      // 229     $ E5
    CPU_OP(implied, UNIMPLEMENTED),      // 230     $ E6
    CPU_OP(implied, UNIMPLEMENTED),      // 231     $ E7
    CPU_OP(implied, UNIMPLEMENTED),      // 232     $ E8
    CPU_OP(implied, UNIMPLEMENTED),      // 233     $ E9
    CPU_OP(implied, UNIMPLEMENTED),      // 234     $ EA
    CPU_OP(implied, UNIMPLEMENTED),      // 235     $ EB
    CPU_OP(implied, UNIMPLEMENTED),      // 236     $ EC
    CPU_OP(implied, UNIMPLEMENTED),      // 237     $ ED
    CPU_OP(implied, UNIMPLEMENTED),      // 238     $ EE
    CPU_OP(implied, UNIMPLEMENTED),      // 239     $ EF
    CPU_OP(implied, UNIMPLEMENTED),      // 240     $ F0
    CPU_OP(implied, UNIMPLEMENTED),      // 241     $ F1
    CPU_OP(implied, UNIMPLEMENTED),      // 242     $ F2
    CPU_OP(implied, UNIMPLEMENTED),      // 243     $ F3
    CPU_OP(implied, UNIMPLEMENTED),      // 244     $ F4
    CPU_OP(implied, UNIMPLEMENTED),      // 245     $ F5
    CPU_OP(implied, UNIMPLEMENTED),      // 246     $ F6
    CPU_OP(implied, UNIMPLEMENTED),      // 247     $ F7
    CPU_OP(implied, UNIMPLEMENTED),      // 248     $ F8
    CPU_OP(implied, UNIMPLEMENTED),      // 249     $ F9
    CPU_OP(implied, UNIMPLEMENTED),      // 250     $ FA
    CPU_OP(implied, UNIMPLEMENTED),      // 251     $ FB
    CPU_OP(implied, UNIMPLEMENTED),      // 252     $ FC
    CPU_OP(implied, UNIMPLEMENTED),      // 253     $ FD
    CPU_OP(implied, UNIMPLEMENTED),      // 254     $ FE
    CPU_OP(implied, UNIMPLEMENTED)       // 255     $ FF
};

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// Addressing modes

ADDRESS_MODE(implied)
{
    return nullptr;
}

ADDRESS_MODE(immediate)
{
    return cpu.fetch_byte_ref( cpu.regs.PC++ );
}

ADDRESS_MODE(absolute)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    return cpu.fetch_byte_ref( lo, hi );
}

ADDRESS_MODE(zero_page)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    return cpu.fetch_byte_ref( lo, 0x00 );
}

ADDRESS_MODE(index_x)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t new_lo = cpu.regs.X + lo;
    if (new_lo < lo || modify)
    { // Page crossing
        hi++;
        cpu.tick_clock();
    }
    return cpu.fetch_byte_ref( new_lo, hi );
}

ADDRESS_MODE(index_y)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t new_lo = cpu.regs.Y + lo;
    if (new_lo < lo || modify)
    { // Page crossing
        hi++;
        cpu.tick_clock();
    }
    return cpu.fetch_byte_ref( new_lo, hi );
}

ADDRESS_MODE(index_zp_x)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    return cpu.fetch_byte_ref( lo + cpu.regs.X, 0x00 );
}

ADDRESS_MODE(index_zp_y)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    return cpu.fetch_byte_ref( lo + cpu.regs.Y, 0x00 );
}

ADDRESS_MODE(indirect)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    uint16_t new_address = ((uint16_t)hi << 8) | lo;
    lo = cpu.fetch_byte( new_address     );
    hi = cpu.fetch_byte( new_address + 1 );
    return cpu.fetch_byte_ref( lo, hi );
}

ADDRESS_MODE(pre_index_indirect_x)
{
    uint8_t tmp = cpu.fetch_byte( cpu.regs.PC++ );
    tmp += cpu.regs.X;
    uint8_t lo = cpu.fetch_byte( tmp,     0x00 );
    uint8_t hi = cpu.fetch_byte( tmp + 1, 0x00 );
    return cpu.fetch_byte_ref( lo, hi );
}

ADDRESS_MODE(post_index_indirect_y)
{
    uint8_t tmp = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t lo = cpu.fetch_byte( tmp,     0x00 );
    uint8_t hi = cpu.fetch_byte( tmp + 1, 0x00 );
    uint8_t new_lo = lo + cpu.regs.Y;
    if (new_lo < lo || modify)
    { // Page crossing
        hi++;
        cpu.tick_clock();
    }
    return cpu.fetch_byte_ref( lo, hi );
}

ADDRESS_MODE(relative)
{
    // TODO(xxx): Remember to add extra clock cycle if branch
    //            causes page movement
    int8_t offset = cpu.fetch_byte( cpu.regs.PC++ );
    return cpu.fetch_byte_ref( cpu.regs.PC + offset );
}

ADDRESS_MODE(accumulator)
{
    return &(cpu.regs.A);
}

////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// OPs
OP_FUNCTION(UNIMPLEMENTED)
{
    printf("OP-CODE UNIMPLEMENTED!\n");
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
    uint8_t operand = *addr_mode(cpu, false);
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
    uint8_t operand = *addr_mode(cpu, false);
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
    uint8_t* operand = addr_mode(cpu, true);
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

OP_FUNCTION(LDA)
{
    uint8_t operand = *addr_mode(cpu, false);
    cpu.regs.A = operand;
    CALC_Z_FLAG( operand );
    CALC_N_FLAG( operand) ;
}



} // nes