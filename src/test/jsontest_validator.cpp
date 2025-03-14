#include "test/jsontest_validator.hpp"
#include "nes.hpp"
#include "logging.hpp"

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include <sstream>
#include <iostream>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

namespace nes
{

void jsontest_validator::init(emu_t* emu_ref, const char* path)
{
    emu = emu_ref;
    
    struct stat path_stat;
    stat(path, &path_stat);
    
    if (S_ISDIR(path_stat.st_mode))
    {
        DIR* dir = opendir(path);
        if (dir != nullptr)
        {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr)
            {
                std::string file_name = entry->d_name;
                if (file_name.find(".json") != std::string::npos)
                {
                    std::string full_path = std::string(path) + "/" + file_name;
                    json_list.push_back(full_path);
                }
            }
            closedir(dir);
        }
    }
    else
    { // Path is not a directory. Treat as single test
        json_list.push_back(path);
    }

    // Sort tests ($00 -> $FF)
    std::sort(json_list.begin(), json_list.end());
}

RESULT jsontest_validator::run_tests()
{
    std::ifstream file;
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
        
        int passed_tests = 0;
        for (rapidjson::SizeType i = 0; i < tests.Size(); ++i)
        { // Loop through tests
            
            // Reset memory
            memset(emu->memory->memory_hook, 0, 0xFFFF);
            bus_activities.clear();
            emu->cpu.trapped = false;

            // Setup CPU regs and memory
            const rapidjson::Value& initial = tests[i]["initial"];
            const rapidjson::Value& final_v = tests[i]["final"];
            const rapidjson::Value& cycles  = tests[i]["cycles"];
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

            // Setup CPU vectors
            emu->cpu.vectors.NMI = emu->cpu.peek_short( 0xFFFA );
            emu->cpu.vectors.RESET = emu->cpu.peek_short( 0xFFFC );
            emu->cpu.vectors.IRQBRK = emu->cpu.peek_short( 0xFFFE );

            // Run test
            uint16_t cycles_executed = 0;
            while (cycles_executed < cycles.Size())
            {
                cycles_executed += emu->cpu.execute();
            }

            // Check results
            bool failure = false;
            if (cycles_executed > cycles.Size()) {
                LOG_E("cycles mismatch! %u != %u", cycles_executed, cycles.Size());
                failure = true;
            }

            for (rapidjson::SizeType j = 0; j < cycles.Size(); ++j)
            {
                if (j >= bus_activities.size()) {
                    LOG_D("Missing bus activity!");
                    failure = true;
                    continue;
                }

                cpu_mem_t::bus_activity_t &activity = bus_activities[j];
                if (cycles[j][0].GetUint() != activity.address) {
                    LOG_D("CYCLE %u ADR %02X != %02X", j+1, activity.address, cycles[j][0].GetUint());
                    failure = true;
                }
                if (cycles[j][1].GetUint() != activity.value) {
                    LOG_D("CYCLE %u VAL %02X != %02X", j+1, activity.value, cycles[j][1].GetUint());
                    failure = true;
                }
                if (strcmp(cycles[j][2].GetString(), "read") == 0) {
                    if (!activity.read) {
                        LOG_D("CYCLE %u STS write != read", j+1);
                        failure = true;
                    }
                } else {
                    if (activity.read) {
                        LOG_D("CYCLE %u STS read != write", j+1);
                        failure = true;
                    }
                }
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
                LOG_D("     N V - B D I Z C");
                LOG_D("Got: %u %u %u %u %u %u %u %u     (%02X)", 
                    emu->cpu.regs.N, emu->cpu.regs.V, 
                    emu->cpu.regs.B >> 1, emu->cpu.regs.B & 0x1, 
                    emu->cpu.regs.D, emu->cpu.regs.I, 
                    emu->cpu.regs.Z, emu->cpu.regs.C, emu->cpu.regs.SR);
                LOG_D("Exp: %u %u %u %u %u %u %u %u     (%02X)", 
                    (p & 0x80) > 0, (p & 0x40) > 0, 
                    (p & 0x20) > 0, (p & 0x10) > 0, 
                    (p & 0x8) > 0, (p & 0x4) > 0, 
                    (p & 0x2) > 0, (p & 0x1) > 0, p);
                failure = true;
            }

            const rapidjson::Value& final_ram = tests[i]["final"]["ram"];
            for (rapidjson::SizeType j = 0; j < final_ram.Size(); ++j)
            {
                uint16_t addr = final_ram[j][0].GetUint();
                uint8_t data = emu->memory->cpu_memory_read(addr, true);
                uint8_t expected = final_ram[j][1].GetUint();
                if (data != expected)
                {
                    LOG_D("Memory mismatch @ %04X  %02X != %02X", addr, data, expected);
                    failure = true;
                }
            }
            
            if (failure)
            {
                LOG_E("Failed test %d: %s (%d/%d tests passed)", passed_tests+1, tests[i]["name"].GetString(), passed_tests, tests.Size());
                return RESULT::RESULT_ERROR;
            }
            passed_tests++;
        }
        LOG_S("%s (%d/%d tests passed)", path.c_str(), passed_tests, tests.Size());
    }
    
    return RESULT::RESULT_OK;
}

mem_dummy_t::mem_dummy_t()
{
    if (memory_hook) delete[] memory_hook;
    memory_hook = new uint8_t[0xFFFF]{0};
}

mem_dummy_t::~mem_dummy_t()
{
    if (memory_hook) delete[] memory_hook;
    memory_hook = nullptr;
}

uint8_t* mem_dummy_t::fetch_byte_ref( uint16_t address )
{
    return &memory_hook[ address ];
}

uint8_t mem_dummy_t::cpu_memory_read( uint16_t address, bool peek )
{
    return memory_hook[address];
}

void mem_dummy_t::cpu_memory_write( uint8_t data, uint16_t address )
{
    memory_hook[address] = data;
}

} // nes