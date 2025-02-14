#include "test/jsontest_validator.hpp"
#include "nes.hpp"
#include "logging.hpp"

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include <sstream>
#include <iostream>
#include <filesystem>

namespace nes
{

void jsontest_validator::init(emu_t* emu_ref, const char* path)
{
    emu = emu_ref;
    // Find tests in directory and store in list
    if (std::filesystem::is_directory(path))
    {
        for (const auto& entry : std::filesystem::directory_iterator(path)) 
        {
            std::filesystem::path file_path = entry.path();
            std::string file_name = file_path.string();
            
            if (file_name.find(".json") != std::string::npos) 
            { // Run test on file
                json_list.push_back(file_name);
            }
        }
    } else 
    { // Path is not a directory. Treat as single test
        json_list.push_back(path);
    }

    // Sort tests ($00 -> $FF)
    std::sort(json_list.begin(),json_list.end());
}

RESULT jsontest_validator::run_tests()
{
    std::ifstream file{};
    for (std::string &path : json_list) 
    {
        file.open(path);
        if (!file.is_open())
        {
            LOG_E("Failed to open json test '%s'", path.c_str());
            throw RESULT_ERROR;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
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

            // Setup vectors
            emu->cpu.vectors.NMI = emu->cpu.peek_short( 0xFFFA );
            emu->cpu.vectors.RESET = emu->cpu.peek_short( 0xFFFC );
            emu->cpu.vectors.IRQBRK = emu->cpu.peek_short( 0xFFFE );

            // Run test
            uint16_t cycles = emu->cpu.execute();

            // Check results
            bool failure = false;
            if (cycles != tests[i]["cycles"].Size()) {
                LOG_E("cycles missmatch! %u != %u", cycles, tests[i]["cycles"].Size());
                failure = true;
            }
            if (emu->cpu.regs.PC != final_v["pc"].GetUint()) {
                LOG_D("PC %02X != %02X", emu->cpu.regs.PC, final_v["pc"].GetUint());
                failure = true;
            }
            if (emu->cpu.regs.SP != final_v["s"].GetUint()) {
                LOG_D("SP %02X != %02X", emu->cpu.regs.SP, final_v["s"].GetUint());
                failure = true;
            }
            if (emu->cpu.regs.A != final_v["a"].GetUint()) {
                LOG_D("A %02X != %02X", emu->cpu.regs.A, final_v["a"].GetUint());
                failure = true;
            }
            if (emu->cpu.regs.X != final_v["x"].GetUint()) {
                LOG_D("X %02X != %02X", emu->cpu.regs.X, final_v["x"].GetUint());
                failure = true;
            }
            if (emu->cpu.regs.Y != final_v["y"].GetUint()) {
                LOG_D("Y %02X != %02X", emu->cpu.regs.Y, final_v["y"].GetUint());
                failure = true;
            }
            uint8_t p = final_v["p"].GetUint();
            if (emu->cpu.regs.SR != final_v["p"].GetUint()) {
                LOG_D("SR %02X != %02X", emu->cpu.regs.SR, p);
                //  7 6 5 4 3 2 1 0
                //  N V - - D I Z C
                if (emu->cpu.regs.N != (p & 0x80))
                {
                    LOG_W("N");
                }
                if (emu->cpu.regs.V != (p & 0x40))
                {
                    LOG_W("V");
                }
                if (emu->cpu.regs.B != (p & 0x30))
                {
                    LOG_W("B");
                }
                if (emu->cpu.regs.D != (p & 0x8))
                {
                    LOG_W("D");
                }
                if (emu->cpu.regs.I != (p & 0x4))
                {
                    LOG_W("I");
                }
                if (emu->cpu.regs.Z != (p & 0x2))
                {
                    LOG_W("Z");
                }
                if (emu->cpu.regs.C != (p & 0x1))
                {
                    LOG_W("C");
                }
                failure = true;
            }
            if (failure)
            {
                LOG_E("Failed test: %s", tests[i]["name"].GetString());
                return RESULT::RESULT_ERROR;
            }
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

uint8_t* mem_dummy_t::fetch_byte_ref( uint16_t address )
{
    return &memory_hook[ address ];
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