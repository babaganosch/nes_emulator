#ifndef NES_VALIDATOR_H
#define NES_VALIDATOR_H

#include "nes.hpp"

#include <fstream>

namespace nes
{

const size_t emu_output_len = 95;

class validator
{
public:
    validator() = default;
    ~validator();

    RESULT init(emu_t* emu_ref, const char* key_path);
    RESULT execute();

private:
    RESULT construct_output_pre_line();
    RESULT construct_output_post_line();
    RESULT validate_line();

    emu_t* emu;
    char emu_output[emu_output_len];
    uint16_t post_fix_cursor{0};
    uint16_t post_fix_letters{0};
    uint32_t line_number{0};
    std::ifstream key{};
};


} // nes

#endif /* NES_VALIDATOR_H */