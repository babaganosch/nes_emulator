#ifndef NES_VALIDATOR_HPP
#define NES_VALIDATOR_HPP

#include "nes.hpp"

#include <fstream>

namespace nes
{

const size_t emu_output_len = 96;

class validator
{
public:
    validator() = default;
    ~validator();

    RESULT init(emu_t* emu_ref, const char* key_path, bool validate_log);
    RESULT execute();

    uint16_t validated_lines{0};

private:
    RESULT construct_output_pre_line();
    RESULT construct_output_post_line();
    RESULT validate_line();

    emu_t* emu;
    bool validate_log{false};
    char emu_output[emu_output_len];
    uint16_t fails{0};
    uint16_t post_fix_cursor{0};
    uint16_t post_fix_letters{0};
    uint64_t line_number{0};
    std::ifstream key{};
};


} // nes

#endif /* NES_VALIDATOR_HPP */