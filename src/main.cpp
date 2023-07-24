#include <cstdio>

#include "nes.hpp"
#include "nes_validator.hpp"

namespace 
{
const char* nes_test_rom = "../data/nestest.nes";
const char* nes_test_log = "../data/nestest.log";
} // anonymous

int main()
{
    printf("--- Enter Main ---\n");
    nes::RESULT ret = nes::RESULT_OK;

    nes::ines_rom_t rom{};
    nes::emu_t emu{};

    rom.load_from_file(nes_test_rom);
    emu.init(rom);
    
    if (true)
    { // NesTest Validation
        nes::validator validator{};
        ret = validator.init(&emu, nes_test_log);

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

    printf("--- Shutting Down ---\n");

    printf("Exiting with code %d\n", ret);
    return ret;
}