#pragma once

#include "typedefs.hpp"

namespace RUPS {
    extern u8 IF;
    extern u8 IE;
}

class reg {
public:
    u8 hi, lo;
    u16 val();
    void set(u16);
};

extern int flag;

extern u16 LCDC;
extern u16 STAT;
extern u16 SCY;
extern u16 SCX;
extern u16 LY;
extern u16 LYC;
extern u16 DMA;
extern u16 BGP;

namespace CPU {
    extern u16 PC;
    extern u16 SP;
    extern u8 IME;
    extern bool halt;
    extern bool halt_bug;
    extern int cycles;
    extern int timing;
    extern int count;

    void executeNextOperation();
    void runOPCode(u8 op_code);
    void init_registers_to_skip_boot();
    void print_registers();

    void push_onto_stack(u16 val);
    u16 pop_from_stack();

    void write(u8 val, u16 addr);
    u8 read();
    void init_opcodes();
    void init_decoder();
    void init();

    int getCyles();
    int getTiming();
    void DMA_routine();

    void cb_not_imp();
    void op_not_imp();
    void STAT();

    void run_cb(u8);
}