#include <cstdio>

#include "nes.hpp"
#include "nes_validator.hpp"

namespace 
{
const char* nes_test_rom = "../data/nestest.nes";
} // anonymous

int main(int argc, char *argv[])
{
    bool validate = false;
    const char* rom_filepath = nes_test_rom;
    const char* validate_log_filepath;
    for ( auto i = 1; i < argc; ++i )
    {
        if ( strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--validate") == 0 )
        {
            if (i + 1 >= argc)
            {
                printf("Invalid arguments\n");
                return nes::RESULT_INVALID_ARGUMENTS;
            }
            validate = true;
            validate_log_filepath = argv[++i];
            continue;
        }

        if ( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 )
        {
            printf("\n");
            printf("Usage: nes_emulator <rom_path>\n");
            printf("       nes_emulator <rom_path> -v <validation_log_path>\n");
            return nes::RESULT_OK;
        }

        else
        {
            rom_filepath = argv[i];
            continue;
        }
    }

    printf("--- NESscape ---\n");
    nes::RESULT ret = nes::RESULT_OK;

    nes::ines_rom_t rom{};
    nes::emu_t emu{};

    try
    {
        rom.load_from_file(rom_filepath);
        emu.init(rom);

        if (validate)
        { // NesTest Validation
            nes::validator validator{};
            ret = validator.init(&emu, validate_log_filepath);

            while (ret > 0)
            {
                ret = validator.execute();
            }
            printf("\nValidated \033[0;34m%u\033[0;0m lines\n", validator.validated_lines);
            
            if (ret == nes::RESULT_VALIDATION_SUCCESS)
            {
                printf("\033[1;32mVALIDATION SUCCESS\033[0;0m\n\n");
                ret = nes::RESULT_OK;
            }
        }
        else
        { // Regular Execution
            while (ret > 0)
            {
                ret = emu.step(1);
            }
        }
    }
    catch(const nes::RESULT& e)
    {
        ret = e;
    }

    printf("--- Shutting Down ---\n");

    printf("Exiting with code %d\n", ret);
    return ret;
}