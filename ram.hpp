#include <string>
#include "typedefs.hpp"

namespace RAM {
    u8 readAt(u16);
    u8 read();
    void write(u8, u16);
    void init_ram(std::string rom_path);
    void dump_oam();
    void DMA_routine();
    void d_vram();
};