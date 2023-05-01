#include <string>
#include "typedefs.hpp"

namespace RAM {
    extern u8 DIV;

    u8 readAt(u16);
    u8 read();
    void write(u8 val, u16 addr);
    void init_ram(std::string rom_path);
    void dump_oam();
    void DMA_routine();
    void d_vram();

    extern bool UP, DOWN, LEFT, RIGHT, A, B, START, SELECT, ACTIONS, DIRECTIONS;
};