#include "cpu.hpp"
#include "input.hpp"
#include "render.hpp"
#include "ram.hpp"
#include "game.hpp"
#include "interrupts.hpp"
#include "ppu.hpp"
#include "sprite_renderer.hpp"

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

void tick(RunOptions options, PPU* ppu) {

    if (options.LOG_STATE || INPUTS::toggle_logging) {
        CPU::print_registers();
    }

    CPU::executeNextOperation();

    ppu->update(CPU::getCyles());
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
    }

    if (options.SKIP_BOOT) {
        CPU::init_registers_to_skip_boot();
    }
}

void game_loop(RunOptions options) {
    initialiseState(options);

    PPU* ppu = buildPPU();

    int input_time = 0;
    int cnt = 0;

    bool render_next_vblank = true;

    while (!INPUTS::getQuit()) {
        input_time++;

        if (input_time == 100) {
            input_time = 0;
            handle_inputs();
        }
        tick(options, ppu);

        if (!options.NO_DISPLAY) {
            u8 y_line = RAM::readAt(LY);
            if (y_line == 144 && render_next_vblank) {
                render_next_vblank = false;

                RENDER::drawFromPPUBuffer(ppu->getBuffer());
                RENDER::drawFrame();
                display_sprites();
                displayObjects();
            }

            if (y_line != 144 && !render_next_vblank) {
                render_next_vblank = true;
            }
        }
    }
}
