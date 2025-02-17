#ifndef JSONTEST_VALIDATOR_HPP
#define JSONTEST_VALIDATOR_HPP

#include "nes.hpp"
#include "logging.hpp"

#include <fstream>
#include <vector>
#include <string>

namespace nes
{

class jsontest_validator
{
public:
    jsontest_validator() = default;

    void init(emu_t* emu_ref, const char* path);
    RESULT run_tests();

private:
    emu_t* emu{nullptr};
    std::vector<std::string> json_list;
};

struct mem_dummy_t : public mem_t
{
    mem_dummy_t();
    ~mem_dummy_t();

    uint8_t* fetch_byte_ref( uint16_t address ) override;

    uint8_t cpu_memory_read( uint16_t address, bool peek ) override;
    void cpu_memory_write( uint8_t data, uint16_t address ) override;
};


} // nes

#endif /* JSONTEST_VALIDATOR_HPP */