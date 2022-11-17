#include <string>
#include "typedefs.hpp"

namespace RAM {
    u8 readAt(u16);
    u8 read();
    void write(u8 val, u16 addr);
    void init_ram(std::string rom_path);
    void dump_oam();
    void DMA_routine();
    void d_vram();
};