#include "cpu.h"
#include "input.hpp"
#include "render.hpp"
#include "ram.hpp"
#include "game.hpp"
#include "interrupts.hpp"

void game_loop(RunOptions options) {
    CPU::init();
    RAM::init_ram(options.romPath);

    if (!options.NO_DISPLAY) {
        RENDER::init();
        RENDER::drawFrame();
        LCD::draw_BG();
    }
    
    if (options.SKIP_BOOT) {
        CPU::init_registers();
        RAM::write(0x04, 0xFF0F);
    }

    u16 debug = 0;
    u8 opcode = 0;

    int inp_time = 0;
    int cnt = 0;

    while (!INPUTS::getQuit()) {
        inp_time++;

        if (inp_time == 100) {
            inp_time = 0;
            for (int i = 0; i < 50; i++) {
                INPUTS::readInputs();
            }

            if (INPUTS::switch_display) {
                INPUTS::switch_display = false;
                if (RENDER::display_mode == GB) {
                    RENDER::setDisplay(SPRITE);;
                } else if (RENDER::display_mode == SPRITE) {
                    RENDER::setDisplay(MEMORY);
                } else if (RENDER::display_mode == MEMORY) {
                    RENDER::setDisplay(GB);
                }
            }
        }

        if (CPU::PC == 0x00FE) {
            if (!options.NO_DISPLAY) {
                LCD::draw_BG(); 
                RENDER::drawFrame();
            }
        }

        if (!CPU::halt) {
            if (options.LOG_STATE) {
                CPU::print_registers();
            }
            
            if (CPU::halt_bug) {
                CPU::halt_bug = false;
                u16 prevPC = CPU::PC;
                opcode = CPU::read();
                CPU::runOPCode(opcode);
                CPU::PC = prevPC;
            } else {
                opcode = CPU::read();
                CPU::runOPCode(opcode);
            }
        }

        if (!options.NO_DISPLAY) {
            LCD::update();
        }  

        TIMER::update();

        handle_interrupts();
    }
}
