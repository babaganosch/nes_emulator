#include <chrono>
#include <cstdio>
#include <cstring>
#include <MiniFB.h>

#include "logging.hpp"
#include "nes.hpp"
#include "render.hpp"
#include "test/jsontest_validator.hpp"
#include "test/nestest_validator.hpp"

namespace 
{
constexpr const char* nes_test_rom = "../thirdparty/nes-test-roms/other/nestest.nes";
constexpr const uint16_t screen_multiplier = 3u;
constexpr const uint16_t microseconds_per_frame = 16667;

#define DEBUG_DRAW_INPUT(input) (input > 0 ? '*' : ' ')

bool json_test = false;
bool validate = false;
bool validate_log = false;
bool debug = false;

float emu_speed = 1.0;

void keyboard_callback(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed)
{
    nes::emu_t* emu = (nes::emu_t*)mfb_get_user_data(window);

    switch ( key )
    {
        case KB_KEY_Z:     emu->memory->gamepad[0].A      = isPressed; break;
        case KB_KEY_X:     emu->memory->gamepad[0].B      = isPressed; break;
        case KB_KEY_N:     emu->memory->gamepad[0].select = isPressed; break;
        case KB_KEY_M:     emu->memory->gamepad[0].start  = isPressed; break;
        case KB_KEY_UP:    emu->memory->gamepad[0].up     = isPressed; break;
        case KB_KEY_DOWN:  emu->memory->gamepad[0].down   = isPressed; break;
        case KB_KEY_LEFT:  emu->memory->gamepad[0].left   = isPressed; break;
        case KB_KEY_RIGHT: emu->memory->gamepad[0].right  = isPressed; break;

        case KB_KEY_0: emu_speed = 0.00; break;
        case KB_KEY_1: emu_speed = 0.33; break;
        case KB_KEY_2: emu_speed = 0.50; break;
        case KB_KEY_3: emu_speed = 0.66; break;
        case KB_KEY_4: emu_speed = 1.00; break;
        case KB_KEY_5: emu_speed = 1.33; break;
        case KB_KEY_6: emu_speed = 1.50; break;
        case KB_KEY_7: emu_speed = 1.66; break;
        case KB_KEY_8: emu_speed = 2.00; break;

        default: break;
    }

    if ( isPressed ) return; // Only react on key release below
    if ( key == KB_KEY_ESCAPE )
    {
        mfb_close(window);
    }

}

} // anonymous

int main(int argc, char *argv[])
{
    const char* rom_filepath = nes_test_rom;
    const char* validate_log_filepath;
    const char* json_test_filepath;
    for ( auto i = 1; i < argc; ++i )
    {
        if ( strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--validate") == 0 )
        {
            validate = true;
            if (i + 1 < argc)
            {
                validate_log = true;
                validate_log_filepath = argv[++i];
            }
            continue;
        }

        if ( strcmp(argv[i], "-j") == 0 )
        {
            json_test = true;
            if (i + 1 < argc)
            {
                json_test_filepath = argv[++i];
                continue;
            } else {
                printf("Missing argument with path to JSON test\n");
                return nes::RESULT_INVALID_ARGUMENTS;
            }
        }

        if ( strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0 )
        {
            debug = true;
            printf("Debug mode ON\n");
            continue;
        }

        if ( strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 )
        {
            printf("\n");
            printf("Usage: nes_emulator <rom_path> <flag>\n");
            printf("Flags:\n");
            printf("       no flag          (regular execution)\n");
            printf("       -d | --debug     (debug execution)\n");
            printf("       -h | --help      (print this help)\n");
            printf("       -v | --validate  (validation execution)\n");
            printf("       -v <validation_log_path>  (validate against provided log file)\n");
            printf("       -j <path to json test>    (validate CPU against JSON test)\n");
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
        if (json_test)
        { // Json Tests
            nes::jsontest_validator validator{};
            emu.init_testsuite(&validator);
            validator.init( &emu, json_test_filepath );
            ret = validator.run_tests();
        } 
        else if (validate)
        { // NesTest Validation
            rom.load_from_file(rom_filepath);
            emu.init(rom);

            nes::nestest_validator validator{};
            ret = validator.init( &emu, validate_log_filepath, validate_log ) ;

            while ( ret == nes::RESULT_OK )
            {
                ret = validator.execute();
            }
            printf("\nValidated \033[0;34m%u\033[0;0m lines\n", validator.validated_lines);
            
            if (ret == nes::RESULT_VALIDATION_SUCCESS)
            {
                printf("\033[1;32mVALIDATION SUCCESS\033[0;0m\n\n");
            }
        }
        else
        { // Regular Execution
            rom.load_from_file(rom_filepath);
            emu.init(rom);

            struct mfb_window *window = 0x0;
            struct mfb_window *nt_window = 0x0;
            nes::clear_framebuffer( emu.front_buffer, 255, 0, 0 );
            nes::clear_framebuffer( emu.back_buffer, 255, 0, 0 );
            
            window = mfb_open_ex( "NesScape", NES_WIDTH * screen_multiplier, NES_HEIGHT * screen_multiplier, WF_RESIZABLE );
            mfb_set_target_fps( 60 );
            mfb_set_user_data( window, (void*)&emu );
            mfb_set_keyboard_callback(window, keyboard_callback);

            if (debug)
            {
                nt_window = mfb_open_ex( "Nametables", NES_WIDTH * 2, NES_HEIGHT * 2, WF_RESIZABLE );
                nes::clear_framebuffer( nes::nt_window_buffer, 255, 0, 0 );
            }

            auto start = std::chrono::high_resolution_clock::now();
            auto time = std::chrono::microseconds(0);

            do
            {
                auto end = std::chrono::high_resolution_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                time += std::chrono::microseconds(microseconds_per_frame) - elapsed;
                start = end;

                // Run NES one frame (about 29781 cycles per frame for 60 FPS (16.67 ms per frame, not 16 flat))
                if (time > std::chrono::microseconds(microseconds_per_frame))
                { // Updating too fast, skip a frame
                    time -= std::chrono::microseconds(microseconds_per_frame);
                } else if (time < std::chrono::microseconds(0)) 
                { // Window minimized or something, reset the correction
                    time = std::chrono::microseconds(0);
                } else 
                {
                    emu.step_cycles(29780 * emu_speed);
                }

                if (debug)
                {
                    nes::cpu_t::regs_t& regs = emu.cpu.regs;
                    nes::draw_text( emu.front_buffer, 1, 1,  "PC   A  X  Y  SR SP CYC");
                    nes::draw_text( emu.front_buffer, 1, 10, 
                        "%04X %02X %02X %02X %02X %02X %08X",
                        regs.PC, regs.A, regs.X, regs.Y, regs.SR, regs.SP, emu.cpu.cycles);
                    nes::draw_text( emu.front_buffer, 1, 19, "EMU %d%%", (int)(emu_speed*100));
                    nes::draw_text( emu.front_buffer, 30, NES_HEIGHT - 10, 
                        "A%c B%c SE%c ST%c U%c D%c L%c R%c",
                        DEBUG_DRAW_INPUT(emu.memory->gamepad[0].A),
                        DEBUG_DRAW_INPUT(emu.memory->gamepad[0].B),
                        DEBUG_DRAW_INPUT(emu.memory->gamepad[0].select),
                        DEBUG_DRAW_INPUT(emu.memory->gamepad[0].start),
                        DEBUG_DRAW_INPUT(emu.memory->gamepad[0].up),
                        DEBUG_DRAW_INPUT(emu.memory->gamepad[0].down),
                        DEBUG_DRAW_INPUT(emu.memory->gamepad[0].left),
                        DEBUG_DRAW_INPUT(emu.memory->gamepad[0].right));

                    nes::clear_framebuffer( nes::nt_window_buffer, 255, 0, 0 );
                    nes::dump_nametables(emu);
                    
                    if ( mfb_update_ex( nt_window, nes::nt_window_buffer, NES_WIDTH * 2, NES_HEIGHT * 2) < 0 ) break;
                }

                if ( mfb_update_ex( window, emu.front_buffer, NES_WIDTH, NES_HEIGHT ) < 0 ) break;
            } while (mfb_wait_sync( window ));
            mfb_close( window );
            printf("Exiting gracefully...\n");
        }
    }
    catch(const nes::RESULT& e)
    {
        ret = e;
    }

    printf("--- Shutting Down ---\n");

    printf("Exiting with code %d %s\n", ret, RESULT_to_string(ret));
    return ret;
}
