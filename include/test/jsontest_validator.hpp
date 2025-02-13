#ifndef JSONTEST_VALIDATOR_HPP
#define JSONTEST_VALIDATOR_HPP

#include "nes.hpp"
#include "logging.hpp"

#include <fstream>

namespace nes
{

class jsontest_validator
{
public:
    jsontest_validator() = default;
    ~jsontest_validator();

    RESULT init(emu_t* emu_ref, const char* test_path);
    void   setup();

private:
    emu_t* emu;
    std::ifstream file{};
};

struct mem_dummy_t : public mem_t
{
    mem_dummy_t();
    ~mem_dummy_t();

    uint8_t cpu_memory_read( uint16_t address, bool peek ) override;
    void cpu_memory_write( uint8_t data, uint16_t address ) override;
};


} // nes

#endif /* JSONTEST_VALIDATOR_HPP */