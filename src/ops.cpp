#include "nes_ops.hpp"
#include "nes.hpp"
#include "logging.hpp"

#define CALC_Z_FLAG(VALUE) cpu.regs.Z = (((VALUE & 0xFF) == 0x00) ? 1 : 0 )
#define CALC_N_FLAG(VALUE) cpu.regs.N = (((VALUE & 0x80) > 0x00) ? 1 : 0 )
#define CALC_C_FLAG(VALUE) cpu.regs.C = (( VALUE > 0xFF ) ? 1 : 0 )
#define CALC_V_FLAG(M, N, RESULT) cpu.regs.V = ((((M ^ RESULT) & (N ^ RESULT) & 0x80) != 0) ? 1 : 0 )

namespace nes
{

namespace
{
void branch( cpu_t &cpu, uint8_t condition, uint8_t expected, uint16_t target )
{
    uint16_t old_PC = cpu.regs.PC;
    if (condition == expected)
    { // Branch occurs
        cpu.tick_clock();
        if ( (old_PC & 0xFF00) != (target & 0xFF00) )
        { // Branch caused page change
            cpu.tick_clock();
        }
        cpu.regs.PC = target;
    }
}

const bool I = false; // Illegal  OP
const bool O = true;  // Official OP

} // anonymous

op_code_t op_codes[256] = {
    CPU_OP(BRK, O, implied),                //   0     $ 00
    CPU_OP(ORA, O, pre_index_indirect_x),   //   1     $ 01
    CPU_OP(JAM, I, implied),                //   2     $ 02
    CPU_OP(SLO, I, pre_index_indirect_x),   //   3     $ 03
    CPU_OP(NOP, I, zero_page),              //   4     $ 04
    CPU_OP(ORA, O, zero_page),              //   5     $ 05
    CPU_OP(ASL, O, zero_page),              //   6     $ 06
    CPU_OP(SLO, I, zero_page),              //   7     $ 07
    CPU_OP(PHP, O, implied),                //   8     $ 08
    CPU_OP(ORA, O, immediate),              //   9     $ 09
    CPU_OP(ASL, O, accumulator),            //  10     $ 0A
    CPU_OP(ANC, I, immediate),              //  11     $ 0B
    CPU_OP(NOP, I, absolute),               //  12     $ 0C
    CPU_OP(ORA, O, absolute),               //  13     $ 0D
    CPU_OP(ASL, O, absolute),               //  14     $ 0E
    CPU_OP(SLO, I, absolute),               //  15     $ 0F
    CPU_OP(BPL, O, relative),               //  16     $ 10
    CPU_OP(ORA, O, post_index_indirect_y),  //  17     $ 11
    CPU_OP(JAM, I, implied),                //  18     $ 12
    CPU_OP(SLO, I, post_index_indirect_y),  //  19     $ 13
    CPU_OP(NOP, I, index_zp_x),             //  20     $ 14
    CPU_OP(ORA, O, index_zp_x),             //  21     $ 15
    CPU_OP(ASL, O, index_zp_x),             //  22     $ 16
    CPU_OP(SLO, I, index_zp_x),             //  23     $ 17
    CPU_OP(CLC, O, implied),                //  24     $ 18
    CPU_OP(ORA, O, index_y),                //  25     $ 19
    CPU_OP(NOP, I, implied),                //  26     $ 1A
    CPU_OP(SLO, I, index_y),                //  27     $ 1B
    CPU_OP(NOP, I, index_x),                //  28     $ 1C
    CPU_OP(ORA, O, index_x),                //  29     $ 1D
    CPU_OP(ASL, O, index_x),                //  30     $ 1E
    CPU_OP(SLO, I, index_x),                //  31     $ 1F
    CPU_OP(JSR, O, absolute),               //  32     $ 20
    CPU_OP(AND, O, pre_index_indirect_x),   //  33     $ 21
    CPU_OP(JAM, I, implied),                //  34     $ 22
    CPU_OP(RLA, I, pre_index_indirect_x),   //  35     $ 23
    CPU_OP(BIT, O, zero_page),              //  36     $ 24
    CPU_OP(AND, O, zero_page),              //  37     $ 25
    CPU_OP(ROL, O, zero_page),              //  38     $ 26
    CPU_OP(RLA, I, zero_page),              //  39     $ 27
    CPU_OP(PLP, O, implied),                //  40     $ 28
    CPU_OP(AND, O, immediate),              //  41     $ 29
    CPU_OP(ROL, O, accumulator),            //  42     $ 2A
    CPU_OP(ANC, I, immediate),              //  43     $ 2B
    CPU_OP(BIT, O, absolute),               //  44     $ 2C
    CPU_OP(AND, O, absolute),               //  45     $ 2D
    CPU_OP(ROL, O, absolute),               //  46     $ 2E
    CPU_OP(RLA, I, absolute),               //  47     $ 2F
    CPU_OP(BMI, O, relative),               //  48     $ 30
    CPU_OP(AND, O, post_index_indirect_y),  //  49     $ 31
    CPU_OP(JAM, I, implied),                //  50     $ 32
    CPU_OP(RLA, I, post_index_indirect_y),  //  51     $ 33
    CPU_OP(NOP, I, index_zp_x),             //  52     $ 34
    CPU_OP(AND, O, index_zp_x),             //  53     $ 35
    CPU_OP(ROL, O, index_zp_x),             //  54     $ 36
    CPU_OP(RLA, I, index_zp_x),             //  55     $ 37
    CPU_OP(SEC, O, implied),                //  56     $ 38
    CPU_OP(AND, O, index_y),                //  57     $ 39
    CPU_OP(NOP, I, implied),                //  58     $ 3A
    CPU_OP(RLA, I, index_y),                //  59     $ 3B
    CPU_OP(NOP, I, index_x),                //  60     $ 3C
    CPU_OP(AND, O, index_x),                //  61     $ 3D
    CPU_OP(ROL, O, index_x),                //  62     $ 3E
    CPU_OP(RLA, I, index_x),                //  63     $ 3F
    CPU_OP(RTI, O, implied),                //  64     $ 40
    CPU_OP(EOR, O, pre_index_indirect_x),   //  65     $ 41
    CPU_OP(JAM, I, implied),                //  66     $ 42
    CPU_OP(SRE, I, pre_index_indirect_x),   //  67     $ 43
    CPU_OP(NOP, I, zero_page),              //  68     $ 44
    CPU_OP(EOR, O, zero_page),              //  69     $ 45
    CPU_OP(LSR, O, zero_page),              //  70     $ 46
    CPU_OP(SRE, I, zero_page),              //  71     $ 47
    CPU_OP(PHA, O, implied),                //  72     $ 48
    CPU_OP(EOR, O, immediate),              //  73     $ 49
    CPU_OP(LSR, O, accumulator),            //  74     $ 4A
    CPU_OP(ALR, I, immediate),              //  75     $ 4B
    CPU_OP(JMP, O, absolute),               //  76     $ 4C
    CPU_OP(EOR, O, absolute),               //  77     $ 4D
    CPU_OP(LSR, O, absolute),               //  78     $ 4E
    CPU_OP(SRE, I, absolute),               //  79     $ 4F
    CPU_OP(BVC, O, relative),               //  80     $ 50
    CPU_OP(EOR, O, post_index_indirect_y),  //  81     $ 51
    CPU_OP(JAM, I, implied),                //  82     $ 52
    CPU_OP(SRE, I, post_index_indirect_y),  //  83     $ 53
    CPU_OP(NOP, I, index_zp_x),             //  84     $ 54
    CPU_OP(EOR, O, index_zp_x),             //  85     $ 55
    CPU_OP(LSR, O, index_zp_x),             //  86     $ 56
    CPU_OP(SRE, I, index_zp_x),             //  87     $ 57
    CPU_OP(CLI, O, implied),                //  88     $ 58
    CPU_OP(EOR, O, index_y),                //  89     $ 59
    CPU_OP(NOP, I, implied),                //  90     $ 5A
    CPU_OP(SRE, I, index_y),                //  91     $ 5B
    CPU_OP(NOP, I, index_x),                //  92     $ 5C
    CPU_OP(EOR, O, index_x),                //  93     $ 5D
    CPU_OP(LSR, O, index_x),                //  94     $ 5E
    CPU_OP(SRE, I, index_x),                //  95     $ 5F
    CPU_OP(RTS, O, implied),                //  96     $ 60
    CPU_OP(ADC, O, pre_index_indirect_x),   //  97     $ 61
    CPU_OP(JAM, I, implied),                //  98     $ 62
    CPU_OP(RRA, I, pre_index_indirect_x),   //  99     $ 63
    CPU_OP(NOP, I, zero_page),              // 100     $ 64
    CPU_OP(ADC, O, zero_page),              // 101     $ 65
    CPU_OP(ROR, O, zero_page),              // 102     $ 66
    CPU_OP(RRA, I, zero_page),              // 103     $ 67
    CPU_OP(PLA, O, implied),                // 104     $ 68
    CPU_OP(ADC, O, immediate),              // 105     $ 69
    CPU_OP(ROR, O, accumulator),            // 106     $ 6A
    CPU_OP(___, O, implied),                // 107     $ 6B
    CPU_OP(JMP, O, indirect),               // 108     $ 6C
    CPU_OP(ADC, O, absolute),               // 109     $ 6D
    CPU_OP(ROR, O, absolute),               // 110     $ 6E
    CPU_OP(RRA, I, absolute),               // 111     $ 6F
    CPU_OP(BVS, O, relative),               // 112     $ 70
    CPU_OP(ADC, O, post_index_indirect_y),  // 113     $ 71
    CPU_OP(JAM, I, implied),                // 114     $ 72
    CPU_OP(RRA, I, post_index_indirect_y),  // 115     $ 73
    CPU_OP(NOP, I, index_zp_x),             // 116     $ 74
    CPU_OP(ADC, O, index_zp_x),             // 117     $ 75
    CPU_OP(ROR, O, index_zp_x),             // 118     $ 76
    CPU_OP(RRA, I, index_zp_x),             // 119     $ 77
    CPU_OP(SEI, O, implied),                // 120     $ 78
    CPU_OP(ADC, O, index_y),                // 121     $ 79
    CPU_OP(NOP, I, implied),                // 122     $ 7A
    CPU_OP(RRA, I, index_y),                // 123     $ 7B
    CPU_OP(NOP, I, index_x),                // 124     $ 7C
    CPU_OP(ADC, O, index_x),                // 125     $ 7D
    CPU_OP(ROR, O, index_x),                // 126     $ 7E
    CPU_OP(RRA, I, index_x),                // 127     $ 7F
    CPU_OP(NOP, I, immediate),              // 128     $ 80
    CPU_OP(STA, O, pre_index_indirect_x),   // 129     $ 81
    CPU_OP(NOP, I, immediate),              // 130     $ 82
    CPU_OP(SAX, I, pre_index_indirect_x),   // 131     $ 83
    CPU_OP(STY, O, zero_page),              // 132     $ 84
    CPU_OP(STA, O, zero_page),              // 133     $ 85
    CPU_OP(STX, O, zero_page),              // 134     $ 86
    CPU_OP(SAX, I, zero_page),              // 135     $ 87
    CPU_OP(DEY, O, implied),                // 136     $ 88
    CPU_OP(NOP, I, immediate),              // 137     $ 89
    CPU_OP(TXA, O, implied),                // 138     $ 8A
    CPU_OP(___, O, implied),                // 139     $ 8B
    CPU_OP(STY, O, absolute),               // 140     $ 8C
    CPU_OP(STA, O, absolute),               // 141     $ 8D
    CPU_OP(STX, O, absolute),               // 142     $ 8E
    CPU_OP(SAX, I, absolute),               // 143     $ 8F
    CPU_OP(BCC, O, relative),               // 144     $ 90
    CPU_OP(STA, O, post_index_indirect_y),  // 145     $ 91
    CPU_OP(JAM, I, implied),                // 146     $ 92
    CPU_OP(___, O, implied),                // 147     $ 93
    CPU_OP(STY, O, index_zp_x),             // 148     $ 94
    CPU_OP(STA, O, index_zp_x),             // 149     $ 95
    CPU_OP(STX, O, index_zp_y),             // 150     $ 96
    CPU_OP(SAX, I, index_zp_y),             // 151     $ 97
    CPU_OP(TYA, O, implied),                // 152     $ 98
    CPU_OP(STA, O, index_y),                // 153     $ 99
    CPU_OP(TXS, O, implied),                // 154     $ 9A
    CPU_OP(___, O, implied),                // 155     $ 9B
    CPU_OP(___, O, implied),                // 156     $ 9C
    CPU_OP(STA, O, index_x),                // 157     $ 9D
    CPU_OP(___, O, implied),                // 158     $ 9E
    CPU_OP(___, O, implied),                // 159     $ 9F
    CPU_OP(LDY, O, immediate),              // 160     $ A0
    CPU_OP(LDA, O, pre_index_indirect_x),   // 161     $ A1
    CPU_OP(LDX, O, immediate),              // 162     $ A2
    CPU_OP(LAX, I, pre_index_indirect_x),   // 163     $ A3
    CPU_OP(LDY, O, zero_page),              // 164     $ A4
    CPU_OP(LDA, O, zero_page),              // 165     $ A5
    CPU_OP(LDX, O, zero_page),              // 166     $ A6
    CPU_OP(LAX, I, zero_page),              // 167     $ A7
    CPU_OP(TAY, O, implied),                // 168     $ A8
    CPU_OP(LDA, O, immediate),              // 169     $ A9
    CPU_OP(TAX, O, implied),                // 170     $ AA
    CPU_OP(___, O, implied),                // 171     $ AB
    CPU_OP(LDY, O, absolute),               // 172     $ AC
    CPU_OP(LDA, O, absolute),               // 173     $ AD
    CPU_OP(LDX, O, absolute),               // 174     $ AE
    CPU_OP(LAX, I, absolute),               // 175     $ AF
    CPU_OP(BCS, O, relative),               // 176     $ B0
    CPU_OP(LDA, O, post_index_indirect_y),  // 177     $ B1
    CPU_OP(JAM, I, implied),                // 178     $ B2
    CPU_OP(LAX, I, post_index_indirect_y),  // 179     $ B3
    CPU_OP(LDY, O, index_zp_x),             // 180     $ B4
    CPU_OP(LDA, O, index_zp_x),             // 181     $ B5
    CPU_OP(LDX, O, index_zp_y),             // 182     $ B6
    CPU_OP(LAX, I, index_zp_y),             // 183     $ B7
    CPU_OP(CLV, O, implied),                // 184     $ B8
    CPU_OP(LDA, O, index_y),                // 185     $ B9
    CPU_OP(TSX, O, implied),                // 186     $ BA
    CPU_OP(___, O, implied),                // 187     $ BB
    CPU_OP(LDY, O, index_x),                // 188     $ BC
    CPU_OP(LDA, O, index_x),                // 189     $ BD
    CPU_OP(LDX, O, index_y),                // 190     $ BE
    CPU_OP(LAX, I, index_y),                // 191     $ BF
    CPU_OP(CPY, O, immediate),              // 192     $ C0
    CPU_OP(CMP, O, pre_index_indirect_x),   // 193     $ C1
    CPU_OP(NOP, I, immediate),              // 194     $ C2
    CPU_OP(DCP, I, pre_index_indirect_x),   // 195     $ C3
    CPU_OP(CPY, O, zero_page),              // 196     $ C4
    CPU_OP(CMP, O, zero_page),              // 197     $ C5
    CPU_OP(DEC, O, zero_page),              // 198     $ C6
    CPU_OP(DCP, I, zero_page),              // 199     $ C7
    CPU_OP(INY, O, implied),                // 200     $ C8
    CPU_OP(CMP, O, immediate),              // 201     $ C9
    CPU_OP(DEX, O, implied),                // 202     $ CA
    CPU_OP(___, O, implied),                // 203     $ CB
    CPU_OP(CPY, O, absolute),               // 204     $ CC
    CPU_OP(CMP, O, absolute),               // 205     $ CD
    CPU_OP(DEC, O, absolute),               // 206     $ CE
    CPU_OP(DCP, I, absolute),               // 207     $ CF
    CPU_OP(BNE, O, relative),               // 208     $ D0
    CPU_OP(CMP, O, post_index_indirect_y),  // 209     $ D1
    CPU_OP(JAM, I, implied),                // 210     $ D2
    CPU_OP(DCP, I, post_index_indirect_y),  // 211     $ D3
    CPU_OP(NOP, I, index_zp_x),             // 212     $ D4
    CPU_OP(CMP, O, index_zp_x),             // 213     $ D5
    CPU_OP(DEC, O, index_zp_x),             // 214     $ D6
    CPU_OP(DCP, I, index_zp_x),             // 215     $ D7
    CPU_OP(CLD, O, implied),                // 216     $ D8
    CPU_OP(CMP, O, index_y),                // 217     $ D9
    CPU_OP(NOP, I, implied),                // 218     $ DA
    CPU_OP(DCP, I, index_y),                // 219     $ DB
    CPU_OP(NOP, I, index_x),                // 220     $ DC
    CPU_OP(CMP, O, index_x),                // 221     $ DD
    CPU_OP(DEC, O, index_x),                // 222     $ DE
    CPU_OP(DCP, I, index_x),                // 223     $ DF
    CPU_OP(CPX, O, immediate),              // 224     $ E0
    CPU_OP(SBC, O, pre_index_indirect_x),   // 225     $ E1
    CPU_OP(NOP, I, immediate),              // 226     $ E2
    CPU_OP(ISB, I, pre_index_indirect_x),   // 227     $ E3
    CPU_OP(CPX, O, zero_page),              // 228     $ E4
    CPU_OP(SBC, O, zero_page),              // 229     $ E5
    CPU_OP(INC, O, zero_page),              // 230     $ E6
    CPU_OP(ISB, I, zero_page),              // 231     $ E7
    CPU_OP(INX, O, implied),                // 232     $ E8
    CPU_OP(SBC, O, immediate),              // 233     $ E9
    CPU_OP(NOP, O, implied),                // 234     $ EA
    CPU_OP(SBC, I, immediate),              // 235     $ EB
    CPU_OP(CPX, O, absolute),               // 236     $ EC
    CPU_OP(SBC, O, absolute),               // 237     $ ED
    CPU_OP(INC, O, absolute),               // 238     $ EE
    CPU_OP(ISB, I, absolute),               // 239     $ EF
    CPU_OP(BEQ, O, relative),               // 240     $ F0
    CPU_OP(SBC, O, post_index_indirect_y),  // 241     $ F1
    CPU_OP(JAM, I, implied),                // 242     $ F2
    CPU_OP(ISB, I, post_index_indirect_y),  // 243     $ F3
    CPU_OP(NOP, I, index_zp_x),             // 244     $ F4
    CPU_OP(SBC, O, index_zp_x),             // 245     $ F5
    CPU_OP(INC, O, index_zp_x),             // 246     $ F6
    CPU_OP(ISB, I, index_zp_x),             // 247     $ F7
    CPU_OP(SED, O, implied),                // 248     $ F8
    CPU_OP(SBC, O, index_y),                // 249     $ F9
    CPU_OP(NOP, I, implied),                // 250     $ FA
    CPU_OP(ISB, I, index_y),                // 251     $ FB
    CPU_OP(NOP, I, index_x),                // 252     $ FC
    CPU_OP(SBC, O, index_x),                // 253     $ FD
    CPU_OP(INC, O, index_x),                // 254     $ FE
    CPU_OP(ISB, I, index_x)                 // 255     $ FF
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
    uint16_t address = UINT16( lo, hi );

    if (cpu.nestest_validation)
    {
        if (!is_branch)
        {
            snprintf(cpu.nestest_validation_str, 5, "= %02X", cpu.peek_byte(address));
        }
        else
        {
            snprintf(cpu.nestest_validation_str, 5, "    ");
        }
    }

    return address;
}

ADDRESS_MODE(zero_page)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint16_t address = UINT16( lo, 0x00 );

    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 3, "%02X", cpu.peek_byte(address));
    }

    return address;
}

ADDRESS_MODE(index_x)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t new_lo = lo + cpu.regs.X;
    uint8_t new_hi = hi;

    new_hi += (new_lo < lo) ? 1 : 0; // Page crossing
    if ( (new_hi != hi) || modify_memory )
    { // Extra cycle for page crossing
        cpu.tick_clock();
    }

    uint16_t address = UINT16( new_lo, new_hi );
    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 10, "%04X = %02X", address, cpu.peek_byte(address));
    }

    return address;
}

ADDRESS_MODE(index_y)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t new_lo = lo + cpu.regs.Y;
    uint8_t new_hi = hi;

    new_hi += (new_lo < lo) ? 1 : 0; // Page crossing
    if ( (new_hi != hi) || modify_memory )
    { // Extra cycle for page crossing
        cpu.tick_clock();
    }

    uint16_t address = UINT16( new_lo, new_hi );
    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 10, "%04X = %02X", address, cpu.peek_byte(address));
    }

    return address;
}

ADDRESS_MODE(index_zp_x)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    lo += cpu.regs.X;
    cpu.tick_clock();

    uint16_t address = UINT16( lo, 0x00 );
    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 8, "%02X = %02X", lo, cpu.peek_byte(address));
    }

    return address;
}

ADDRESS_MODE(index_zp_y)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    lo += cpu.regs.Y;
    cpu.tick_clock();

    uint16_t address = UINT16( lo, 0x00 );
    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 8, "%02X = %02X", lo, cpu.peek_byte(address));
    }

    return address;
}

ADDRESS_MODE(indirect)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    
    /* 6502 JMP indirect bug:
    * There's a bug originally on the 6502 chip with the indirect JMP
    * instruction. The bug manifests itself when a page crossing occurs
    * during fetching of the high byte of the indirect address.
    * I.e. if the address to fetch from is lo: $xxFF, hi: $xxFF + 1.
    * This will cause the low byte to wrap around to 00 and not
    * modify the high byte causing the indirect address to be fetched
    * from $xxFF and $xx00.
    * 
    * This bug is replicated in this implementation.
    */
    uint8_t ind_lo = cpu.fetch_byte( lo,     hi );
    uint8_t ind_hi = cpu.fetch_byte( lo + 1, hi );

    uint16_t address = UINT16( ind_lo, ind_hi );
    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 5, "%04X", address);
    }
    return address;
}

ADDRESS_MODE(pre_index_indirect_x)
{
    uint8_t tmp = cpu.fetch_byte( cpu.regs.PC++ );
    tmp += cpu.regs.X;
    cpu.tick_clock();
    uint8_t lo = cpu.fetch_byte( tmp,     0x00 );
    uint8_t hi = cpu.fetch_byte( tmp + 1, 0x00 );
    uint16_t address = UINT16( lo, hi );

    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 15, "%02X = %04X = %02X", tmp, address, cpu.peek_byte(address));
    }

    return address;
}

ADDRESS_MODE(post_index_indirect_y)
{
    uint8_t tmp = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t lo  = cpu.fetch_byte( tmp,     0x00 );
    uint8_t hi  = cpu.fetch_byte( tmp + 1, 0x00 );
    uint8_t new_lo = lo + cpu.regs.Y;
    uint8_t new_hi = hi;

    new_hi += (new_lo < lo) ? 1 : 0; // Page crossing
    if ( (new_hi != hi) || modify_memory )
    { // Extra cycle for page crossing
        cpu.tick_clock();
    }

    uint16_t address = UINT16( new_lo, new_hi );
    if (cpu.nestest_validation)
    {
        snprintf(cpu.nestest_validation_str, 17, "%04X @ %04X = %02X", UINT16( lo, hi ), address, cpu.peek_byte(address));
    }

    return address;
}

ADDRESS_MODE(relative)
{
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
    LOG_E("UNIMPLEMENTED OP-CODE ENCOUNTERED!");
    throw;
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
    uint16_t address = addr_mode( cpu, false, false );
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
    uint16_t address = addr_mode( cpu, false, false );
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
    uint8_t* operand = nullptr;
    uint16_t address = addr_mode( cpu, true, false );
    if (addr_mode != addr_mode_accumulator)
    {
        operand = cpu.fetch_byte_ref( address );
        cpu.tick_clock(); // Extra cycle when modifying value
    }
    else
    {
        operand = &cpu.regs.A;
    }
    if (operand)
    {
        uint16_t data = *operand << 1;
    
        CALC_N_FLAG( data );
        CALC_Z_FLAG( data );
        CALC_C_FLAG( data );
        cpu.write_byte( data, operand );
    }
    else
    {
        cpu.tick_clock();
    }
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
    uint16_t address   = addr_mode( cpu, false, true );
    branch( cpu, cpu.regs.C, 0, address );
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
    uint16_t address = addr_mode( cpu, false, true );
    branch( cpu, cpu.regs.C, 1, address );
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
    uint16_t address   = addr_mode( cpu, false, true );
    branch( cpu, cpu.regs.Z, 1, address );
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
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t operand = cpu.fetch_byte( address );
    cpu.regs.N = BIT_CHECK_HI( operand, 7 );
    cpu.regs.V = BIT_CHECK_HI( operand, 6 ); 
    cpu.regs.Z = CALC_Z_FLAG( operand & cpu.regs.A );
}

/////////////////////////////////////////////////////////
// BMI - Branch on Result Minus
// branch on N = 1
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(BMI)
{
    uint16_t address = addr_mode( cpu, false, true );
    branch( cpu, cpu.regs.N, 1, address );
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
    uint16_t address   = addr_mode( cpu, false, true );
    branch( cpu, cpu.regs.Z, 0, address );
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
    uint16_t address   = addr_mode( cpu, false, true );
    branch( cpu, cpu.regs.N, 0, address );
}

/////////////////////////////////////////////////////////
// BRK - Force Break
// BRK initiates a software interrupt similar to a hardware
// interrupt (IRQ). The return address pushed to the stack is
// PC+2, providing an extra byte of spacing for a break mark
// (identifying a reason for the break.)
// The status register will be pushed to the stack with the break
// flag set to 1. However, when retrieved during RTI or by a PLP
// instruction, the break flag will be ignored.
// The interrupt disable flag is not set automatically.
//
// interrupt, push PC+2, push SR
//
// N Z C I D V
// - - - 1 - -
//
OP_FUNCTION(BRK)
{
    addr_mode( cpu, true, false );
    cpu.push_short_to_stack( cpu.regs.PC + 1 );
    cpu.push_byte_to_stack( cpu.regs.SR | 0x10 );
    cpu.regs.I  = 1;
    cpu.regs.PC = cpu.vectors.NMI;
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
    uint16_t address   = addr_mode( cpu, false, true );
    branch( cpu, cpu.regs.V, 0, address );
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
    uint16_t address   = addr_mode( cpu, false, true );
    branch( cpu, cpu.regs.V, 1, address );
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
    addr_mode( cpu, false, false );
    cpu.regs.C = 0;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// CLD - Clear Decimal Mode
// 0 -> D
//
// N Z C I D V
// - - - - 0 -
//
OP_FUNCTION(CLD)
{
    addr_mode( cpu, false, false );
    cpu.regs.D = 0;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// CLI - Clear Interrupt Disable Bit
// 0 -> I
//
// N Z C I D V
// - - - 0 - -
//
OP_FUNCTION(CLI)
{
    addr_mode( cpu, false, false );
    cpu.regs.I = 0;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// CLV - Clear Overflow Flag
// 0 -> V
//
// N Z C I D V
// - - - - - 0
//
OP_FUNCTION(CLV)
{
    addr_mode( cpu, false, false );
    cpu.regs.V = 0;
    cpu.tick_clock();
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
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t operand = cpu.fetch_byte( address );
    uint8_t data = cpu.regs.A - operand;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    cpu.regs.C = (cpu.regs.A >= operand) ? 1 : 0;
}

/////////////////////////////////////////////////////////
// CPX - Compare Memory and Index X
// X - M
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(CPX)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t operand  = cpu.fetch_byte( address );
    uint8_t data = cpu.regs.X - operand;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    cpu.regs.C = (cpu.regs.X >= operand) ? 1 : 0;
}

/////////////////////////////////////////////////////////
// CPY - Compare Memory and Index Y
// Y - M
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(CPY)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t operand = cpu.fetch_byte( address );
    uint8_t data = cpu.regs.Y - operand;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    cpu.regs.C = (cpu.regs.Y >= operand) ? 1 : 0;
}

/////////////////////////////////////////////////////////
// DEC - Decrement Memory by One
// M - 1 -> M
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(DEC)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint8_t data = cpu.fetch_byte( address );
    data = data - 0x1;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    cpu.tick_clock();
    cpu.write_byte( data, address );
}

/////////////////////////////////////////////////////////
// DEX - Decrement Index X by One
// X - 1 -> X
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(DEX)
{
    addr_mode( cpu, false, false );
    cpu.regs.X--;
    CALC_N_FLAG( cpu.regs.X );
    CALC_Z_FLAG( cpu.regs.X );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// DEY - Decrement Index Y by One
// Y - 1 -> Y
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(DEY)
{
    addr_mode( cpu, false, false );
    cpu.regs.Y--;
    CALC_N_FLAG( cpu.regs.Y );
    CALC_Z_FLAG( cpu.regs.Y );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// EOR - Exclusive-OR Memory with Accumulator
// A EOR M -> A
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(EOR)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t operand  = cpu.fetch_byte( address );
    cpu.regs.A = operand ^ cpu.regs.A;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
}

/////////////////////////////////////////////////////////
// INC - Increment Memory by One
// M + 1 -> M
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(INC)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint8_t data = cpu.fetch_byte( address );
    data = data + 0x1;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    cpu.tick_clock();
    cpu.write_byte( data, address );
}

/////////////////////////////////////////////////////////
// INX - Increment Index X by One
// X + 1 -> X
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(INX)
{
    addr_mode( cpu, false, false );
    cpu.regs.X++;
    CALC_N_FLAG( cpu.regs.X );
    CALC_Z_FLAG( cpu.regs.X );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// INY - Increment Index Y by One
// Y + 1 -> Y
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(INY)
{
    addr_mode( cpu, false, false );
    cpu.regs.Y++;
    CALC_N_FLAG( cpu.regs.Y );
    CALC_Z_FLAG( cpu.regs.Y );
    cpu.tick_clock();
}

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
    uint16_t address = addr_mode( cpu, false, true );
    cpu.regs.PC = address;
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
    uint16_t address = addr_mode( cpu, false, true );
    cpu.tick_clock(); // One extra cycle when buffering data
    cpu.regs.PC = address;
}

/////////////////////////////////////////////////////////
// LDA - Load Accumulator with Memory
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(LDA)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t  operand = cpu.fetch_byte( address );
    cpu.regs.A = operand;
    CALC_Z_FLAG( operand );
    CALC_N_FLAG( operand );
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
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t operand = cpu.fetch_byte( address );
    cpu.regs.X = operand;
    CALC_Z_FLAG( operand );
    CALC_N_FLAG( operand );
}

/////////////////////////////////////////////////////////
// LDY - Load Index Y with Memory
// M -> Y
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(LDY)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t operand  = cpu.fetch_byte( address );
    cpu.regs.Y = operand;
    CALC_Z_FLAG( operand );
    CALC_N_FLAG( operand );
}

/////////////////////////////////////////////////////////
// LSR - Shift One Bit Right (Memory or Accumulator)
// 0 -> [76543210] -> C
//
// N Z C I D V
// 0 + + - - -
//
OP_FUNCTION(LSR)
{
    uint8_t* operand = &cpu.regs.A;
    uint16_t address = addr_mode( cpu, true, false );
    if (addr_mode != addr_mode_accumulator)
    {
        operand = cpu.fetch_byte_ref( address );
        cpu.tick_clock(); // Extra cycle when modifying value
    }
    if (operand)
    {
        uint16_t data = *operand >> 1;
        cpu.regs.N = 0;
        CALC_Z_FLAG( data );
        cpu.regs.C = (*operand & 0x1) == 0x1;
        cpu.write_byte( data, operand );
    }
    else
    {
        LOG_E("LSR attempted on nullptr");
    }
}

/////////////////////////////////////////////////////////
// NOP, DOP, TOP
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(NOP)
{
    addr_mode( cpu, false, false );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// ORA - OR Memory with Accumulator
// A OR M -> A
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(ORA)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t operand  = cpu.fetch_byte( address );
    cpu.regs.A = operand | cpu.regs.A;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
}

/////////////////////////////////////////////////////////
// PHA - Push Accumulator on Stack
// push A
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(PHA)
{
    addr_mode( cpu, true, false );
    cpu.push_byte_to_stack( cpu.regs.A );
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
    addr_mode( cpu, true, false );
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
    addr_mode( cpu, false, false );
    uint8_t data = cpu.pull_byte_from_stack();
    cpu.tick_clock( 2 ); // Stack-pop extra cycles
    cpu.regs.A = data;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
}

/////////////////////////////////////////////////////////
// PLP - Pull Processor Status from Stack
// The status register will be pulled with the break
// flag and bit 5 ignored.
//
// pull SR
//
// N Z C I D V
// from stack
//
OP_FUNCTION(PLP)
{
    addr_mode( cpu, false, false );
    uint8_t status = cpu.pull_byte_from_stack() & 0xCF;
    cpu.tick_clock( 2 ); // Stack-pop extra cycles
    cpu.regs.SR = status | (cpu.regs.SR & 0x30);
}

/////////////////////////////////////////////////////////
// ROL - Rotate One Bit Left (Memory or Accumulator)
// C <- [76543210] <- C
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(ROL)
{
    uint8_t* operand = &cpu.regs.A;
    uint16_t address = addr_mode( cpu, true, false );
    if (addr_mode != addr_mode_accumulator)
    {
        operand = cpu.fetch_byte_ref( address );
        cpu.tick_clock(); // Extra cycle when modifying value
    }
    if (operand)
    {
        uint16_t data = (uint16_t)*operand << 1;
        data = (data & 0xFFFE) | cpu.regs.C;

        cpu.regs.C = data > 0xFF ? 1 : 0;
        CALC_N_FLAG( data );
        CALC_Z_FLAG( data );
        
        cpu.write_byte( data, operand );
    }
    else
    {
        LOG_E("ROL attempted on nullptr");
    }
}

/////////////////////////////////////////////////////////
// ROR - Rotate One Bit Right (Memory or Accumulator)
// C -> [76543210] -> C
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(ROR)
{
    uint8_t* operand = &cpu.regs.A;
    uint16_t address = addr_mode( cpu, true, false );
    if (addr_mode != addr_mode_accumulator)
    {
        operand = cpu.fetch_byte_ref( address );
        cpu.tick_clock(); // Extra cycle when modifying value
    }
    if (operand)
    {
        uint8_t data = *operand >> 1;
        data = (data & 0x7F) | cpu.regs.C << 7;

        cpu.regs.C = (*operand & 0x1) == 0x1;
        CALC_N_FLAG( data );
        CALC_Z_FLAG( data );
        
        cpu.write_byte( data, operand );
    }
    else
    {
        LOG_E("ROR attempted on nullptr");
    }
}

/////////////////////////////////////////////////////////
// RTI - Return from Interrupt
// The status register is pulled with the break flag
// and bit 5 ignored. Then PC is pulled from the stack.
//
// pull SR, pull PC
//
// N Z C I D V
// from stack
//
OP_FUNCTION(RTI)
{
    addr_mode( cpu, false, false );
    cpu.tick_clock( 2 ); // Stack-pop extra cycles
    uint8_t status = cpu.pull_byte_from_stack();
    cpu.regs.SR = (status & 0xCF) | (cpu.regs.SR & 0x30);
    uint16_t new_pc = cpu.pull_short_from_stack();
    cpu.regs.PC = new_pc;    
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
    addr_mode( cpu, false, true );
    uint16_t address = cpu.pull_short_from_stack();
    cpu.tick_clock( 2 ); // Stack-pop extra cycles
    cpu.regs.PC = address + 1;
    cpu.tick_clock(); // One extra cycle to post-increment PC
}

/////////////////////////////////////////////////////////
// SBC - Subtract Memory from Accumulator with Borrow
//         _
// A - M - C -> A
//
// N Z C I D V
// + + + - - +
//
OP_FUNCTION(SBC)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t  data0    = cpu.fetch_byte( address );
    uint8_t  data1 = ~data0;
    uint16_t res = cpu.regs.A + data1 + cpu.regs.C;

    CALC_C_FLAG( res );
    CALC_Z_FLAG( res );
    CALC_N_FLAG( res );
    CALC_V_FLAG( cpu.regs.A, data1, res );
    cpu.regs.A = res;
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
    addr_mode( cpu, false, false );
    cpu.regs.C = 1;
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
    addr_mode( cpu, false, false );
    cpu.regs.D = 1;
    cpu.tick_clock();
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
    addr_mode( cpu, false, false );
    cpu.regs.I = 1;
    cpu.tick_clock();
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
    uint16_t address = addr_mode( cpu, true, false );
    cpu.write_byte( cpu.regs.A, address );
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
    uint16_t address = addr_mode( cpu, false, false );
    cpu.write_byte( cpu.regs.X, address );
}

/////////////////////////////////////////////////////////
// STY - Store Index Y in Memory
// Y -> M
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(STY)
{
    uint16_t address = addr_mode( cpu, true, false );
    cpu.write_byte( cpu.regs.Y, address );
}

/////////////////////////////////////////////////////////
// TAX - Transfer Accumulator to Index X
// A -> X
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(TAX)
{
    addr_mode( cpu, false, false );
    cpu.regs.X = cpu.regs.A;
    CALC_N_FLAG( cpu.regs.X );
    CALC_Z_FLAG( cpu.regs.X );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// TAY - Transfer Accumulator to Index Y
// A -> Y
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(TAY)
{
    addr_mode( cpu, false, false );
    cpu.regs.Y = cpu.regs.A;
    CALC_N_FLAG( cpu.regs.Y );
    CALC_Z_FLAG( cpu.regs.Y );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// TSX - Transfer Stack Pointer to Index X
// SP -> X
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(TSX)
{
    addr_mode( cpu, false, false );
    cpu.regs.X = cpu.regs.SP;
    CALC_N_FLAG( cpu.regs.X );
    CALC_Z_FLAG( cpu.regs.X );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// TXA - Transfer Index X to Accumulator
// X -> A
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(TXA)
{
    addr_mode( cpu, false, false );
    cpu.regs.A = cpu.regs.X;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// TXS - Transfer Index X to Stack Pointer
// X -> SP
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(TXS)
{
    addr_mode( cpu, false, false );
    cpu.regs.SP = cpu.regs.X;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// TYA - Transfer Index Y to Accumulator
// Y -> A
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(TYA)
{
    addr_mode( cpu, false, false );
    cpu.regs.A = cpu.regs.Y;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
    cpu.tick_clock();
}

///////// ------------------------ Illegal Operations ------------------------

/////////////////////////////////////////////////////////
// ALR (ASR)
// AND oper + LSR
//
// A AND oper, 0 -> [76543210] -> C
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(ALR)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t  operand = cpu.fetch_byte( address );
    uint8_t  data = operand & cpu.regs.A;
    cpu.regs.C = data & 0x1;
    data = data >> 1;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
}

/////////////////////////////////////////////////////////
// LAX - LDA oper + LDX oper
// M -> A -> X
//
// N Z C I D V
// + + - - - -
//
OP_FUNCTION(LAX)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t data = cpu.fetch_byte( address );
    cpu.regs.A = data;
    cpu.regs.X = data;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
}

/////////////////////////////////////////////////////////
// SAX (AXS, AAX)
// A and X are put on the bus at the same time (resulting 
// effectively in an AND operation) and stored in M
//
// A AND X -> M
//
// N Z C I D V
// - - - - - -
//
OP_FUNCTION(SAX)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint8_t data = cpu.regs.A & cpu.regs.X;
    cpu.write_byte( data, address );
}

/////////////////////////////////////////////////////////
// DCP  (DCM)
// DEC oper + CMP oper
//
// M - 1 -> M, A - M
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(DCP)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint8_t  operand = cpu.fetch_byte( address );
    // M - 1 -> M
    uint8_t data = operand - 0x1;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    CALC_C_FLAG( data );
    cpu.write_byte( data, address );
    // A - M
    data = cpu.regs.A - data;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    cpu.regs.C = (cpu.regs.A >= operand) ? 1 : 0;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// ISB (ISC, INS)
// INC oper + SBC oper
//                     _
// M + 1 -> M, A - M - C -> A
//
// N Z C I D V
// + + + - - +
//
OP_FUNCTION(ISB)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint8_t  data0 = cpu.fetch_byte( address );
    // M + 1 -> M
    uint8_t  data1 = data0 + 0x1;
    CALC_N_FLAG( data1 );
    CALC_Z_FLAG( data1 );
    cpu.write_byte( data1, address );
    //         _
    // A - M - C -> A
    data0 = ~data1;
    uint16_t res = cpu.regs.A + data0 + cpu.regs.C;
    CALC_C_FLAG( res );
    CALC_Z_FLAG( res );
    CALC_N_FLAG( res );
    CALC_V_FLAG( cpu.regs.A, data0, res );
    cpu.regs.A = res;
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// SLO (ASO)
// ASL oper + ORA oper
//
// M = C <- [76543210] <- 0, A OR M -> A
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(SLO)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint16_t data    = cpu.fetch_byte( address );
    // M = C <- [76543210] <- 0
    data = data << 1;
    CALC_C_FLAG( data );
    cpu.write_byte( data, address );
    // A OR M -> A
    cpu.regs.A = cpu.regs.A | data;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// RLA
// ROL oper + AND oper
//
// M = C <- [76543210] <- C, A AND M -> A
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(RLA)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint16_t data    = cpu.fetch_byte( address );
    // M = C <- [76543210] <- C
    data = (data << 1) | cpu.regs.C;
    CALC_C_FLAG( data );
    cpu.write_byte( data, address );
    // A AND M -> A
    cpu.regs.A = cpu.regs.A & data;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// SRE (LSE)
// LSR oper + EOR oper
//
// M = 0 -> [76543210] -> C, A EOR M -> A
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(SRE)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint8_t  data    = cpu.fetch_byte( address );
    // M = 0 -> [76543210] -> C
    cpu.regs.C = data & 0x01;
    data = data >> 1;
    cpu.write_byte( data, address );
    // A EOR M -> A
    cpu.regs.A = cpu.regs.A ^ data;
    CALC_N_FLAG( cpu.regs.A );
    CALC_Z_FLAG( cpu.regs.A );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// RRA
// ROR oper + ADC oper
//
// M = C -> [76543210] -> C, A + M + C -> A, C
//
// N Z C I D V
// + + + - - +
//
OP_FUNCTION(RRA)
{
    uint16_t address = addr_mode( cpu, true, false );
    uint8_t  data    = cpu.fetch_byte( address );
    // M = C -> [76543210] -> C
    bool old_C = cpu.regs.C;
    cpu.regs.C = data & 0x01;
    data = (data >> 1) | old_C << 7;
    cpu.write_byte( data, address );
    // A + M + C -> A, C
    uint16_t res = cpu.regs.A + data + cpu.regs.C;
    cpu.regs.A = res;
    CALC_N_FLAG( res );
    CALC_Z_FLAG( res );
    CALC_C_FLAG( res );
    CALC_V_FLAG( cpu.regs.A, data, res );
    cpu.tick_clock();
}

/////////////////////////////////////////////////////////
// ANC (ANC2)
// AND oper + set C as ASL
//
// A AND oper, bit(7) -> C
//
// N Z C I D V
// + + + - - -
//
OP_FUNCTION(ANC)
{
    uint16_t address = addr_mode( cpu, false, false );
    uint8_t  operand = cpu.fetch_byte( address );
    uint8_t  data = operand & cpu.regs.A;
    CALC_N_FLAG( data );
    CALC_Z_FLAG( data );
    cpu.regs.C = BIT_CHECK_HI( data, 7 );
}

/////////////////////////////////////////////////////////
// JAM (KIL, HLT)
// These instructions freeze the CPU.
//
// The processor will be trapped infinitely in T1 phase 
// with $FF on the data bus. — Reset required.
//
OP_FUNCTION(JAM)
{
    LOG_E("JAM occured, dumping state..");
    LOG_E("PC: %04X", cpu.regs.PC);
    LOG_E("SP: %02X", cpu.regs.SP);
    LOG_E("P:  %02X", cpu.regs.SR);
    LOG_E("X:  %02X", cpu.regs.X);
    LOG_E("Y:  %02X", cpu.regs.Y);
    LOG_E("A:  %02X", cpu.regs.A);
    throw RESULT_ERROR;
}

} // nes