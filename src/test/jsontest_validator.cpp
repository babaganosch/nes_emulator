#include "test/jsontest_validator.hpp"
#include "nes.hpp"
#include "logging.hpp"

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include <sstream>
#include <iostream>

namespace nes
{

jsontest_validator::~jsontest_validator()
{
    if (file.is_open())
    {
        file.close();
    }
}

RESULT jsontest_validator::init(emu_t* emu_ref, const char* test_path)
{
    emu = emu_ref;

    file.open(test_path);
    if (!file.is_open())
    {
        LOG_E("Failed to open json test '%s'", test_path);
        throw RESULT_ERROR;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_contents{"{ \"tests\": " + buffer.str() + " }"};

    rapidjson::Document document;
    document.Parse(file_contents.c_str());

    assert(document.IsObject());

    const rapidjson::Value& tests = document["tests"];
    assert(tests.IsArray());
    
    for (rapidjson::SizeType i = 0; i < tests.Size(); ++i)
    { // Loop through tests
        setup();
        const rapidjson::Value& initial = tests[i]["initial"];
        const rapidjson::Value& final_v = tests[i]["final"];
        emu->cpu.regs.PC = initial["pc"].GetUint();
        emu->cpu.regs.SP = initial["s"].GetUint();
        emu->cpu.regs.A  = initial["a"].GetUint();
        emu->cpu.regs.X  = initial["x"].GetUint();
        emu->cpu.regs.Y  = initial["y"].GetUint();
        emu->cpu.regs.SR = initial["p"].GetUint();
        const rapidjson::Value& ram = tests[i]["initial"]["ram"];
        for (rapidjson::SizeType j = 0; j < ram.Size(); ++j)
        {
            emu->memory->cpu_memory_write(ram[j][1].GetUint(), ram[j][0].GetUint());
        }

        // TODO: Seperate step with precise cycles? use CPU execute?
        //std::cout << emu->step_cycles( tests[i]["cycles"].Size() ) << std::endl;
        emu->step_cycles( tests[i]["cycles"].Size() );

        if (emu->cpu.regs.PC != final_v["pc"].GetUint()) {
            LOG_E("ERROR PC");
        }
        if (emu->cpu.regs.SP != final_v["s"].GetUint()) {
            LOG_E("ERROR SP");
        }
        if (emu->cpu.regs.A != final_v["a"].GetUint()) {
            LOG_E("ERROR A");
        }
        if (emu->cpu.regs.X != final_v["x"].GetUint()) {
            LOG_E("ERROR X");
        }
        if (emu->cpu.regs.Y != final_v["y"].GetUint()) {
            LOG_E("ERROR Y");
        }
        if (emu->cpu.regs.SR != final_v["p"].GetUint()) {
            LOG_E("ERROR SR");
        }
    }

    return RESULT::RESULT_OK;
}

void jsontest_validator::setup()
{
    if (!emu->memory->memory_hook)
    {
        delete[] emu->memory->memory_hook;
    }
    emu->memory->memory_hook = new uint8_t[0xFFFF]{0};
}

mem_dummy_t::mem_dummy_t()
{
    memory_hook = new uint8_t[0xFFFF]{0};
}

mem_dummy_t::~mem_dummy_t()
{
    if (memory_hook) delete[] memory_hook;
}

uint8_t mem_dummy_t::cpu_memory_read( uint16_t address, bool peek )
{
    (void) peek;
    return memory_hook[address];
}

void mem_dummy_t::cpu_memory_write( uint8_t data, uint16_t address )
{
    memory_hook[address] = data;
}

} // nes