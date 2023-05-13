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

void tick(RunOptions options) {


    if (options.LOG_STATE || INPUTS::toggle_logging) {
        CPU::print_registers();
    }

    CPU::executeNextOperation();

    if (!options.NO_DISPLAY) {
        LCD::update();
    }

    TIMER::update();

    handle_interrupts();
}

void initialiseState(RunOptions options) {
    if (!options.NO_DISPLAY) {
        RENDER::init();

        if (options.romPath == "") {
            options.romPath = INPUTS::listen_for_dropped_file();
        }
    }

    CPU::init();
    RAM::init_ram(options.romPath);

    if (!options.NO_DISPLAY) {
        RENDER::drawFrame();
        LCD::draw_BG();
    }

    if (options.SKIP_BOOT) {
        CPU::init_registers_to_skip_boot();
    }
}

void game_loop(RunOptions options) {
    initialiseState(options);

    int input_time = 0;
    int cnt = 0;

    while (!INPUTS::getQuit()) {
        input_time++;

        if (input_time == 100) {
            input_time = 0;
            handle_inputs();
        }
        tick(options);
    }
}
