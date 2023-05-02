#include "cpu.hpp"
#include "input.hpp"
#include "render.hpp"
#include "ram.hpp"
#include "game.hpp"
#include "interrupts.hpp"

void handle_inputs() {
    for (int i = 0; i < 50; i++) {
        INPUTS::readInputs();
    }

    if (INPUTS::switch_display) {
        INPUTS::switch_display = false;
        if (RENDER::display_mode == GB) {
            RENDER::setDisplay(SPRITE);;
        } else if (RENDER::display_mode == SPRITE) {
            RENDER::setDisplay(GB);
        }
    }
}

void game_loop(RunOptions options) {
    CPU::init();
    RAM::init_ram(options.romPath);

    if (!options.NO_DISPLAY) {
        RENDER::init();
        RENDER::drawFrame();
        LCD::draw_BG();
    }

    if (options.SKIP_BOOT) {
        CPU::init_registers_to_skip_boot();
        // RAM::write(0x04, 0xFF0F);
    }

    u16 debug = 0;
    u8 opcode = 0;

    int inp_time = 0;
    int cnt = 0;

    while (!INPUTS::getQuit()) {
        inp_time++;

        if (inp_time == 100) {
            inp_time = 0;
            handle_inputs();
        }

        if (options.LOG_STATE) {
            CPU::print_registers();
        }

        CPU::executeNextOperation();

        if (!options.NO_DISPLAY) {
            LCD::update();
        }

        TIMER::update();

        handle_interrupts();
    }
}
