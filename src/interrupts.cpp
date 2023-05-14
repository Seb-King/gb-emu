#include "render.hpp"
#include "utils.hpp"
#include "ram.hpp"
#include "cpu.hpp"
#include "sprite_renderer.hpp"
#include "debug_display.hpp"
#include "ppu.hpp"

namespace TIMER {
    int counter = 0;

    /**
    * The gameboy has a timer with selectable frequency of 4096, 16384 65536, 262144 Hertz
    * when it overflows, it generates an interrupt it is then loaded with the contents of
    * timer modulo (TMA)
    */
    u8 TIMA; // 0xFF05 frequency specified by 0xFF07 
    u8 TMA; // 0xFF06 when TIMA overflows this data is loaded

    // Timer control:
    // Bit 2 = 0 : stop timer, = 1 : start timer
    // Bits 1+0 - Input clock select, = 00 4.096KHz, = 01 : 262.144KHz, = 10 65.536KHz, =11 : 16.384KHz 
    // u8 TAC; 
    // 1024 cycles, 16, 64, 256

    void inc(int amount) {
        u8 t = RAM::readAt(0xFF05);
        TIMA = t + amount;
        if (TIMA < t) {
            u8 mod = RAM::readAt(0xFF06);
            TIMA = mod;
            CPU::write(mod, 0xFF05);
            CPU::write(RAM::readAt(0xFF0F) | 0b00000100, 0xFF0F);
        } else {
            RAM::write(t + amount, 0xFF05);
        }
    }

    void update() {
        RAM::DIV++;

        u8 TAC = RAM::readAt(0xFF07);
        if (((TAC & 0b00000100) == 0b00000100)) {
            counter += CPU::cycles;
            int rate = 0;
            if ((TAC & 0b00000011) == 0) {
                rate = 1024;
            } else if ((TAC & 0b00000011) == 1) {
                rate = 16;
            } else if ((TAC & 0b00000011) == 2) {
                rate = 64;
            } else if ((TAC & 0b00000011) == 3) {
                rate = 256;
            }
            int amount = counter / rate;
            if (amount != 0) {
                counter = counter % rate;
                TIMER::inc(amount);
            }
        }
    }

    void overflow() {
        CPU::SP--;
        CPU::write((CPU::PC & 0xFF00) >> 8, CPU::SP);
        CPU::SP--;
        CPU::write(CPU::PC & 0x00FF, CPU::SP);
        CPU::PC = 0x0050;

        RUPS::IF = RAM::readAt(0xFF0F);
        RUPS::IF &= 0b11111011;
        CPU::write(RUPS::IF, 0xFF0F);
    }
}

void joypadInterrupt() {
    CPU::push_onto_stack(CPU::PC);
    CPU::PC = 0x0060;

    RUPS::IF = RAM::readAt(0xFF0F);
    RUPS::IF &= 0b11101111;
    CPU::write(RUPS::IF, 0xFF0F);
}

void v_blank() {
    u8 STAT = RAM::readAt(0xFF41);
    STAT |= 0b00000001;
    STAT &= 0b11111101;
    CPU::write(STAT, 0xFF41);

    CPU::push_onto_stack(CPU::PC);
    CPU::PC = 0x0040;
    flag = 0;

    RUPS::IF = RAM::readAt(0xFF0F);
    RUPS::IF &= 0b11111110;
    CPU::write(RUPS::IF, 0xFF0F);
}

bool handle_interrupts() {
    RUPS::IF = RAM::readAt(0xFF0F);
    RUPS::IE = RAM::readAt(0xFFFF);

    if ((RUPS::IF & 1) == 1 && (RUPS::IE & 1) == 1) {
        if (CPU::halt) {
            CPU::halt = false;
            CPU::halt_bug = true;
        }

        if (CPU::IME == 0) {
            return false;
        }

        v_blank();
    }

    if ((RUPS::IF & 0x04) == 0x04 && (RUPS::IE & 0x04) == 0x04) {
        if (CPU::halt) {
            CPU::halt = false;
            CPU::halt_bug = true;
        }

        if (CPU::IME == 0) {
            return false;
        }
        TIMER::overflow();

    }

    if ((RUPS::IF & 0x10) == 0x10 && (RUPS::IE & 0x10) == 0x10) {
        if (CPU::halt) {
            CPU::halt = false;
            CPU::halt_bug = true;
        }

        if (CPU::IME == 0) {
            return false;
        }

        joypadInterrupt();
    }
    return true;
}
