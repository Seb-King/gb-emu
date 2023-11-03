#pragma once
#include <string>
#include "typedefs.hpp"
#include "cartridge.hpp"

class RAM {
public:
    RAM();
    u8 DIV;
    u8 readAt(u16);
    void write(u8 val, u16 addr);
    void init_ram(std::string rom_path);
    void dump_oam();
    void DMA_routine();
    void d_vram();

    bool UP, DOWN, LEFT, RIGHT, A, B, START, SELECT, ACTIONS, DIRECTIONS;

private:
    Cartridge* cart;
    u16 LCDC;
    u16 STAT;
    u16 SCY;
    u16 SCX;
    u16 LY;
    u16 LYC;
    u16 DMA;
    u16 BGP;
    u16 ie;

    std::vector<u8> vRam;
    std::vector<u8> iRam;
    std::vector<u8> mbc;
    std::vector<u8> OAM;
    std::vector<u8> i2Ram;
    std::vector<u8> bootRom;
    std::vector<u8> moreRam;

    u8 read_from_cart(u16 addr);
    u8 joyPadInputs();
};
