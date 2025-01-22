#include <cstdio>
#include <MiniFB.h>

#include "nes.hpp"
#include "nes_validator.hpp"
#include "render.hpp"
#include "logging.hpp"

namespace 
{
constexpr const char* nes_test_rom = "../data/nestest.nes";
constexpr const uint16_t screen_multiplier = 3u;

bool validate = false;
bool validate_log = false;
bool debug = false;

void keyboard_callback(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed)
{
    nes::emu_t* emu = (nes::emu_t*)mfb_get_user_data(window);

    switch ( key )
    {
        case KB_KEY_Z:     emu->memory.gamepad[0].A      = isPressed; break;
        case KB_KEY_X:     emu->memory.gamepad[0].B      = isPressed; break;
        case KB_KEY_N:     emu->memory.gamepad[0].select = isPressed; break;
        case KB_KEY_M:     emu->memory.gamepad[0].start  = isPressed; break;
        case KB_KEY_UP:    emu->memory.gamepad[0].up     = isPressed; break;
        case KB_KEY_DOWN:  emu->memory.gamepad[0].down   = isPressed; break;
        case KB_KEY_LEFT:  emu->memory.gamepad[0].left   = isPressed; break;
        case KB_KEY_RIGHT: emu->memory.gamepad[0].right  = isPressed; break;
        default: break;
    }

    if (isPressed) return; // Only react on key release below
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

            struct mfb_window *window = 0x0;
            struct mfb_window *nt_window = 0x0;
            nes::clear_window_buffer( 255, 0, 0 );
            
            window = mfb_open_ex( "NesScape", NES_WIDTH * screen_multiplier, NES_HEIGHT * screen_multiplier, WF_RESIZABLE );
            mfb_set_target_fps( 60 );
            mfb_set_user_data( window, (void*)&emu );
            mfb_set_keyboard_callback(window, keyboard_callback);

            if (debug)
            {
                nt_window = mfb_open_ex( "Nametables", NES_WIDTH * 2, NES_HEIGHT * 2, WF_RESIZABLE );
                nes::clear_nt_window_buffer( 255, 0, 0 );
            }

            do
            {
                nes::clear_window_buffer( 255, 0, 0 );

                // Run NES one frame (about 29786 cycles per frame for 60 FPS)
                //emu.step_vblank();
                emu.step_vblank();
                //emu.step_cycles(29780 / 2);
                //emu.step_cycles(29780);

                if (debug)
                {
                    nes::cpu_t::regs_t& regs = emu.cpu.regs;
                    nes::draw_text( 1, 1,  "PC   A  X  Y  SR SP CYC");
                    nes::draw_text( 1, 10, "%04X %02X %02X %02X %02X %02X %08X",
                                           regs.PC, regs.A, regs.X, regs.Y, regs.SR, regs.SP, emu.cpu.cycles);
                    nes::draw_text( 30, NES_HEIGHT - 10, "A%c B%c SE%c ST%c U%c D%c L%c R%c",
                                           emu.memory.gamepad[0].A > 0 ? '*' : ' ', 
                                           emu.memory.gamepad[0].B > 0 ? '*' : ' ',
                                           emu.memory.gamepad[0].select > 0 ? '*' : ' ',
                                           emu.memory.gamepad[0].start > 0 ? '*' : ' ',
                                           emu.memory.gamepad[0].up > 0 ? '*' : ' ',
                                           emu.memory.gamepad[0].down > 0 ? '*' : ' ',
                                           emu.memory.gamepad[0].left > 0 ? '*' : ' ',
                                           emu.memory.gamepad[0].right > 0 ? '*' : ' ');

                    nes::clear_nt_window_buffer( 255, 0, 0 );
                    nes::dump_nametables(emu);
                    
                    if ( mfb_update_ex( nt_window, nes::nt_window_buffer, NES_WIDTH * 2, NES_HEIGHT * 2) < 0 ) break;
                }

                if ( mfb_update_ex( window, nes::window_buffer, NES_WIDTH, NES_HEIGHT ) < 0 ) break;
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
