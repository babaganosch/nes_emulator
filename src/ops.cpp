#include "nes_ops.hpp"
#include "nes.hpp"

namespace nes
{

op_codes_t op_codes[256] = {
    CPU_OP(absolute, UNIMPLEMENTED), //   0     $ 00
    CPU_OP(absolute, UNIMPLEMENTED), //   1     $ 01
    CPU_OP(absolute, UNIMPLEMENTED), //   2     $ 02
    CPU_OP(absolute, UNIMPLEMENTED), //   3     $ 03
    CPU_OP(absolute, UNIMPLEMENTED), //   4     $ 04
    CPU_OP(absolute, UNIMPLEMENTED), //   5     $ 05
    CPU_OP(absolute, UNIMPLEMENTED), //   6     $ 06
    CPU_OP(absolute, UNIMPLEMENTED), //   7     $ 07
    CPU_OP(absolute, UNIMPLEMENTED), //   8     $ 08
    CPU_OP(absolute, UNIMPLEMENTED), //   9     $ 09
    CPU_OP(absolute, UNIMPLEMENTED), //  10     $ 0A
    CPU_OP(absolute, UNIMPLEMENTED), //  11     $ 0B
    CPU_OP(absolute, UNIMPLEMENTED), //  12     $ 0C
    CPU_OP(absolute, UNIMPLEMENTED), //  13     $ 0D
    CPU_OP(absolute, UNIMPLEMENTED), //  14     $ 0E
    CPU_OP(absolute, UNIMPLEMENTED), //  15     $ 0F
    CPU_OP(absolute, UNIMPLEMENTED), //  16     $ 10
    CPU_OP(absolute, UNIMPLEMENTED), //  17     $ 11
    CPU_OP(absolute, UNIMPLEMENTED), //  18     $ 12
    CPU_OP(absolute, UNIMPLEMENTED), //  19     $ 13
    CPU_OP(absolute, UNIMPLEMENTED), //  20     $ 14
    CPU_OP(absolute, UNIMPLEMENTED), //  21     $ 15
    CPU_OP(absolute, UNIMPLEMENTED), //  22     $ 16
    CPU_OP(absolute, UNIMPLEMENTED), //  23     $ 17
    CPU_OP(absolute, UNIMPLEMENTED), //  24     $ 18
    CPU_OP(absolute, UNIMPLEMENTED), //  25     $ 19
    CPU_OP(absolute, UNIMPLEMENTED), //  26     $ 1A
    CPU_OP(absolute, UNIMPLEMENTED), //  27     $ 1B
    CPU_OP(absolute, UNIMPLEMENTED), //  28     $ 1C
    CPU_OP(absolute, UNIMPLEMENTED), //  29     $ 1D
    CPU_OP(absolute, UNIMPLEMENTED), //  30     $ 1E
    CPU_OP(absolute, UNIMPLEMENTED), //  31     $ 1F
    CPU_OP(absolute, UNIMPLEMENTED), //  32     $ 20
    CPU_OP(absolute, UNIMPLEMENTED), //  33     $ 21
    CPU_OP(absolute, UNIMPLEMENTED), //  34     $ 22
    CPU_OP(absolute, UNIMPLEMENTED), //  35     $ 23
    CPU_OP(absolute, UNIMPLEMENTED), //  36     $ 24
    CPU_OP(absolute, UNIMPLEMENTED), //  37     $ 25
    CPU_OP(absolute, UNIMPLEMENTED), //  38     $ 26
    CPU_OP(absolute, UNIMPLEMENTED), //  39     $ 27
    CPU_OP(absolute, UNIMPLEMENTED), //  40     $ 28
    CPU_OP(absolute, UNIMPLEMENTED), //  41     $ 29
    CPU_OP(absolute, UNIMPLEMENTED), //  42     $ 2A
    CPU_OP(absolute, UNIMPLEMENTED), //  43     $ 2B
    CPU_OP(absolute, UNIMPLEMENTED), //  44     $ 2C
    CPU_OP(absolute, UNIMPLEMENTED), //  45     $ 2D
    CPU_OP(absolute, UNIMPLEMENTED), //  46     $ 2E
    CPU_OP(absolute, UNIMPLEMENTED), //  47     $ 2F
    CPU_OP(absolute, UNIMPLEMENTED), //  48     $ 30
    CPU_OP(absolute, UNIMPLEMENTED), //  49     $ 31
    CPU_OP(absolute, UNIMPLEMENTED), //  50     $ 32
    CPU_OP(absolute, UNIMPLEMENTED), //  51     $ 33
    CPU_OP(absolute, UNIMPLEMENTED), //  52     $ 34
    CPU_OP(absolute, UNIMPLEMENTED), //  53     $ 35
    CPU_OP(absolute, UNIMPLEMENTED), //  54     $ 36
    CPU_OP(absolute, UNIMPLEMENTED), //  55     $ 37
    CPU_OP(absolute, UNIMPLEMENTED), //  56     $ 38
    CPU_OP(absolute, UNIMPLEMENTED), //  57     $ 39
    CPU_OP(absolute, UNIMPLEMENTED), //  58     $ 3A
    CPU_OP(absolute, UNIMPLEMENTED), //  59     $ 3B
    CPU_OP(absolute, UNIMPLEMENTED), //  60     $ 3C
    CPU_OP(absolute, UNIMPLEMENTED), //  61     $ 3D
    CPU_OP(absolute, UNIMPLEMENTED), //  62     $ 3E
    CPU_OP(absolute, UNIMPLEMENTED), //  63     $ 3F
    CPU_OP(absolute, UNIMPLEMENTED), //  64     $ 40
    CPU_OP(absolute, UNIMPLEMENTED), //  65     $ 41
    CPU_OP(absolute, UNIMPLEMENTED), //  66     $ 42
    CPU_OP(absolute, UNIMPLEMENTED), //  67     $ 43
    CPU_OP(absolute, UNIMPLEMENTED), //  68     $ 44
    CPU_OP(absolute, UNIMPLEMENTED), //  69     $ 45
    CPU_OP(absolute, UNIMPLEMENTED), //  70     $ 46
    CPU_OP(absolute, UNIMPLEMENTED), //  71     $ 47
    CPU_OP(absolute, UNIMPLEMENTED), //  72     $ 48
    CPU_OP(absolute, UNIMPLEMENTED), //  73     $ 49
    CPU_OP(absolute, UNIMPLEMENTED), //  74     $ 4A
    CPU_OP(absolute, UNIMPLEMENTED), //  75     $ 4B
    CPU_OP(absolute, UNIMPLEMENTED), //  76     $ 4C
    CPU_OP(absolute, UNIMPLEMENTED), //  77     $ 4D
    CPU_OP(absolute, UNIMPLEMENTED), //  78     $ 4E
    CPU_OP(absolute, UNIMPLEMENTED), //  79     $ 4F
    CPU_OP(absolute, UNIMPLEMENTED), //  80     $ 50
    CPU_OP(absolute, UNIMPLEMENTED), //  81     $ 51
    CPU_OP(absolute, UNIMPLEMENTED), //  82     $ 52
    CPU_OP(absolute, UNIMPLEMENTED), //  83     $ 53
    CPU_OP(absolute, UNIMPLEMENTED), //  84     $ 54
    CPU_OP(absolute, UNIMPLEMENTED), //  85     $ 55
    CPU_OP(absolute, UNIMPLEMENTED), //  86     $ 56
    CPU_OP(absolute, UNIMPLEMENTED), //  87     $ 57
    CPU_OP(absolute, UNIMPLEMENTED), //  88     $ 58
    CPU_OP(absolute, UNIMPLEMENTED), //  89     $ 59
    CPU_OP(absolute, UNIMPLEMENTED), //  90     $ 5A
    CPU_OP(absolute, UNIMPLEMENTED), //  91     $ 5B
    CPU_OP(absolute, UNIMPLEMENTED), //  92     $ 5C
    CPU_OP(absolute, UNIMPLEMENTED), //  93     $ 5D
    CPU_OP(absolute, UNIMPLEMENTED), //  94     $ 5E
    CPU_OP(absolute, UNIMPLEMENTED), //  95     $ 5F
    CPU_OP(absolute, UNIMPLEMENTED), //  96     $ 60
    CPU_OP(absolute, UNIMPLEMENTED), //  97     $ 61
    CPU_OP(absolute, UNIMPLEMENTED), //  98     $ 62
    CPU_OP(absolute, UNIMPLEMENTED), //  99     $ 63
    CPU_OP(absolute, UNIMPLEMENTED), // 100     $ 64
    CPU_OP(absolute, UNIMPLEMENTED), // 101     $ 65
    CPU_OP(absolute, UNIMPLEMENTED), // 102     $ 66
    CPU_OP(absolute, UNIMPLEMENTED), // 103     $ 67
    CPU_OP(absolute, UNIMPLEMENTED), // 104     $ 68
    CPU_OP(absolute, UNIMPLEMENTED), // 105     $ 69
    CPU_OP(absolute, UNIMPLEMENTED), // 106     $ 6A
    CPU_OP(absolute, UNIMPLEMENTED), // 107     $ 6B
    CPU_OP(absolute, UNIMPLEMENTED), // 108     $ 6C
    CPU_OP(absolute, UNIMPLEMENTED), // 109     $ 6D
    CPU_OP(absolute, UNIMPLEMENTED), // 110     $ 6E
    CPU_OP(absolute, UNIMPLEMENTED), // 111     $ 6F
    CPU_OP(absolute, UNIMPLEMENTED), // 112     $ 70
    CPU_OP(absolute, UNIMPLEMENTED), // 113     $ 71
    CPU_OP(absolute, UNIMPLEMENTED), // 114     $ 72
    CPU_OP(absolute, UNIMPLEMENTED), // 115     $ 73
    CPU_OP(absolute, UNIMPLEMENTED), // 116     $ 74
    CPU_OP(absolute, UNIMPLEMENTED), // 117     $ 75
    CPU_OP(absolute, UNIMPLEMENTED), // 118     $ 76
    CPU_OP(absolute, UNIMPLEMENTED), // 119     $ 77
    CPU_OP(absolute, UNIMPLEMENTED), // 120     $ 78
    CPU_OP(absolute, UNIMPLEMENTED), // 121     $ 79
    CPU_OP(absolute, UNIMPLEMENTED), // 122     $ 7A
    CPU_OP(absolute, UNIMPLEMENTED), // 123     $ 7B
    CPU_OP(absolute, UNIMPLEMENTED), // 124     $ 7C
    CPU_OP(absolute, UNIMPLEMENTED), // 125     $ 7D
    CPU_OP(absolute, UNIMPLEMENTED), // 126     $ 7E
    CPU_OP(absolute, UNIMPLEMENTED), // 127     $ 7F
    CPU_OP(absolute, UNIMPLEMENTED), // 128     $ 80
    CPU_OP(absolute, UNIMPLEMENTED), // 129     $ 81
    CPU_OP(absolute, UNIMPLEMENTED), // 130     $ 82
    CPU_OP(absolute, UNIMPLEMENTED), // 131     $ 83
    CPU_OP(absolute, UNIMPLEMENTED), // 132     $ 84
    CPU_OP(absolute, UNIMPLEMENTED), // 133     $ 85
    CPU_OP(absolute, UNIMPLEMENTED), // 134     $ 86
    CPU_OP(absolute, UNIMPLEMENTED), // 135     $ 87
    CPU_OP(absolute, UNIMPLEMENTED), // 136     $ 88
    CPU_OP(absolute, UNIMPLEMENTED), // 137     $ 89
    CPU_OP(absolute, UNIMPLEMENTED), // 138     $ 8A
    CPU_OP(absolute, UNIMPLEMENTED), // 139     $ 8B
    CPU_OP(absolute, UNIMPLEMENTED), // 140     $ 8C
    CPU_OP(absolute, UNIMPLEMENTED), // 141     $ 8D
    CPU_OP(absolute, UNIMPLEMENTED), // 142     $ 8E
    CPU_OP(absolute, UNIMPLEMENTED), // 143     $ 8F
    CPU_OP(absolute, UNIMPLEMENTED), // 144     $ 90
    CPU_OP(absolute, UNIMPLEMENTED), // 145     $ 91
    CPU_OP(absolute, UNIMPLEMENTED), // 146     $ 92
    CPU_OP(absolute, UNIMPLEMENTED), // 147     $ 93
    CPU_OP(absolute, UNIMPLEMENTED), // 148     $ 94
    CPU_OP(absolute, UNIMPLEMENTED), // 149     $ 95
    CPU_OP(absolute, UNIMPLEMENTED), // 150     $ 96
    CPU_OP(absolute, UNIMPLEMENTED), // 151     $ 97
    CPU_OP(absolute, UNIMPLEMENTED), // 152     $ 98
    CPU_OP(absolute, UNIMPLEMENTED), // 153     $ 99
    CPU_OP(absolute, UNIMPLEMENTED), // 154     $ 9A
    CPU_OP(absolute, UNIMPLEMENTED), // 155     $ 9B
    CPU_OP(absolute, UNIMPLEMENTED), // 156     $ 9C
    CPU_OP(absolute, UNIMPLEMENTED), // 157     $ 9D
    CPU_OP(absolute, UNIMPLEMENTED), // 158     $ 9E
    CPU_OP(absolute, UNIMPLEMENTED), // 159     $ 9F
    CPU_OP(absolute, UNIMPLEMENTED), // 160     $ A0
    CPU_OP(absolute, UNIMPLEMENTED), // 161     $ A1
    CPU_OP(absolute, UNIMPLEMENTED), // 162     $ A2
    CPU_OP(absolute, UNIMPLEMENTED), // 163     $ A3
    CPU_OP(absolute, UNIMPLEMENTED), // 164     $ A4
    CPU_OP(absolute, UNIMPLEMENTED), // 165     $ A5
    CPU_OP(absolute, UNIMPLEMENTED), // 166     $ A6
    CPU_OP(absolute, UNIMPLEMENTED), // 167     $ A7
    CPU_OP(absolute, UNIMPLEMENTED), // 168     $ A8
    CPU_OP(immediate, LDA),          // 169     $ A9
    CPU_OP(absolute, UNIMPLEMENTED), // 170     $ AA
    CPU_OP(absolute, UNIMPLEMENTED), // 171     $ AB
    CPU_OP(absolute, UNIMPLEMENTED), // 172     $ AC
    CPU_OP(absolute, LDA),           // 173     $ AD
    CPU_OP(absolute, UNIMPLEMENTED), // 174     $ AE
    CPU_OP(absolute, UNIMPLEMENTED), // 175     $ AF
    CPU_OP(absolute, UNIMPLEMENTED), // 176     $ B0
    CPU_OP(absolute, UNIMPLEMENTED), // 177     $ B1
    CPU_OP(absolute, UNIMPLEMENTED), // 178     $ B2
    CPU_OP(absolute, UNIMPLEMENTED), // 179     $ B3
    CPU_OP(absolute, UNIMPLEMENTED), // 180     $ B4
    CPU_OP(absolute, UNIMPLEMENTED), // 181     $ B5
    CPU_OP(absolute, UNIMPLEMENTED), // 182     $ B6
    CPU_OP(absolute, UNIMPLEMENTED), // 183     $ B7
    CPU_OP(absolute, UNIMPLEMENTED), // 184     $ B8
    CPU_OP(absolute, UNIMPLEMENTED), // 185     $ B9
    CPU_OP(absolute, UNIMPLEMENTED), // 186     $ BA
    CPU_OP(absolute, UNIMPLEMENTED), // 187     $ BB
    CPU_OP(absolute, UNIMPLEMENTED), // 188     $ BC
    CPU_OP(absolute, UNIMPLEMENTED), // 189     $ BD
    CPU_OP(absolute, UNIMPLEMENTED), // 190     $ BE
    CPU_OP(absolute, UNIMPLEMENTED), // 191     $ BF
    CPU_OP(absolute, UNIMPLEMENTED), // 192     $ C0
    CPU_OP(absolute, UNIMPLEMENTED), // 193     $ C1
    CPU_OP(absolute, UNIMPLEMENTED), // 194     $ C2
    CPU_OP(absolute, UNIMPLEMENTED), // 195     $ C3
    CPU_OP(absolute, UNIMPLEMENTED), // 196     $ C4
    CPU_OP(absolute, UNIMPLEMENTED), // 197     $ C5
    CPU_OP(absolute, UNIMPLEMENTED), // 198     $ C6
    CPU_OP(absolute, UNIMPLEMENTED), // 199     $ C7
    CPU_OP(absolute, UNIMPLEMENTED), // 200     $ C8
    CPU_OP(absolute, UNIMPLEMENTED), // 201     $ C9
    CPU_OP(absolute, UNIMPLEMENTED), // 202     $ CA
    CPU_OP(absolute, UNIMPLEMENTED), // 203     $ CB
    CPU_OP(absolute, UNIMPLEMENTED), // 204     $ CC
    CPU_OP(absolute, UNIMPLEMENTED), // 205     $ CD
    CPU_OP(absolute, UNIMPLEMENTED), // 206     $ CE
    CPU_OP(absolute, UNIMPLEMENTED), // 207     $ CF
    CPU_OP(absolute, UNIMPLEMENTED), // 208     $ D0
    CPU_OP(absolute, UNIMPLEMENTED), // 209     $ D1
    CPU_OP(absolute, UNIMPLEMENTED), // 210     $ D2
    CPU_OP(absolute, UNIMPLEMENTED), // 211     $ D3
    CPU_OP(absolute, UNIMPLEMENTED), // 212     $ D4
    CPU_OP(absolute, UNIMPLEMENTED), // 213     $ D5
    CPU_OP(absolute, UNIMPLEMENTED), // 214     $ D6
    CPU_OP(absolute, UNIMPLEMENTED), // 215     $ D7
    CPU_OP(absolute, UNIMPLEMENTED), // 216     $ D8
    CPU_OP(absolute, UNIMPLEMENTED), // 217     $ D9
    CPU_OP(absolute, UNIMPLEMENTED), // 218     $ DA
    CPU_OP(absolute, UNIMPLEMENTED), // 219     $ DB
    CPU_OP(absolute, UNIMPLEMENTED), // 220     $ DC
    CPU_OP(absolute, UNIMPLEMENTED), // 221     $ DD
    CPU_OP(absolute, UNIMPLEMENTED), // 222     $ DE
    CPU_OP(absolute, UNIMPLEMENTED), // 223     $ DF
    CPU_OP(absolute, UNIMPLEMENTED), // 224     $ E0
    CPU_OP(absolute, UNIMPLEMENTED), // 225     $ E1
    CPU_OP(absolute, UNIMPLEMENTED), // 226     $ E2
    CPU_OP(absolute, UNIMPLEMENTED), // 227     $ E3
    CPU_OP(absolute, UNIMPLEMENTED), // 228     $ E4
    CPU_OP(absolute, UNIMPLEMENTED), // 229     $ E5
    CPU_OP(absolute, UNIMPLEMENTED), // 230     $ E6
    CPU_OP(absolute, UNIMPLEMENTED), // 231     $ E7
    CPU_OP(absolute, UNIMPLEMENTED), // 232     $ E8
    CPU_OP(absolute, UNIMPLEMENTED), // 233     $ E9
    CPU_OP(absolute, UNIMPLEMENTED), // 234     $ EA
    CPU_OP(absolute, UNIMPLEMENTED), // 235     $ EB
    CPU_OP(absolute, UNIMPLEMENTED), // 236     $ EC
    CPU_OP(absolute, UNIMPLEMENTED), // 237     $ ED
    CPU_OP(absolute, UNIMPLEMENTED), // 238     $ EE
    CPU_OP(absolute, UNIMPLEMENTED), // 239     $ EF
    CPU_OP(absolute, UNIMPLEMENTED), // 240     $ F0
    CPU_OP(absolute, UNIMPLEMENTED), // 241     $ F1
    CPU_OP(absolute, UNIMPLEMENTED), // 242     $ F2
    CPU_OP(absolute, UNIMPLEMENTED), // 243     $ F3
    CPU_OP(absolute, UNIMPLEMENTED), // 244     $ F4
    CPU_OP(absolute, UNIMPLEMENTED), // 245     $ F5
    CPU_OP(absolute, UNIMPLEMENTED), // 246     $ F6
    CPU_OP(absolute, UNIMPLEMENTED), // 247     $ F7
    CPU_OP(absolute, UNIMPLEMENTED), // 248     $ F8
    CPU_OP(absolute, UNIMPLEMENTED), // 249     $ F9
    CPU_OP(absolute, UNIMPLEMENTED), // 250     $ FA
    CPU_OP(absolute, UNIMPLEMENTED), // 251     $ FB
    CPU_OP(absolute, UNIMPLEMENTED), // 252     $ FC
    CPU_OP(absolute, UNIMPLEMENTED), // 253     $ FD
    CPU_OP(absolute, UNIMPLEMENTED), // 254     $ FE
    CPU_OP(absolute, UNIMPLEMENTED)  // 255     $ FF
};

/////// Addressing modes
ADDRESS_MODE(implied)
{
    return 0;
}

ADDRESS_MODE(immediate)
{
    return cpu.fetch_byte( cpu.regs.PC++ );
}

ADDRESS_MODE(absolute)
{
    uint8_t lo = cpu.fetch_byte( cpu.regs.PC++ );
    uint8_t hi = cpu.fetch_byte( cpu.regs.PC++ );
    return cpu.fetch_byte( lo, hi );
}


/////// OPs
OP_FUNCTION(UNIMPLEMENTED)
{
    printf("OP-CODE UNIMPLEMENTED!\n");
}

OP_FUNCTION(LDA)
{
    cpu.regs.A = operand;
    cpu.regs.Z = operand == 0;
    cpu.regs.N = (operand & 0b1000000) > 0;
}

OP_FUNCTION(LDX)
{

}


} // nes