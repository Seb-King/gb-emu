#include <bitset>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "file_handling.hpp"
#include "typedefs.hpp"
#include "ram.hpp"
#include "cartridge.hpp"

namespace RAM {
    u16 LCDC = 0xFF40;
    u16 STAT = 0xFF41;  // LCDC status (interrupts and memory access)
    u16 SCY = 0xFF42; 	// - Scroll Y
    u16 SCX = 0xFF43; 	// - Scroll X
    u16 LY = 0xFF44;		//0xFF44 - LCDC Y-Coordinate (the vertical line to which present data is transferred to the LCD driver)
    u16 LYC = 0xFF45;		//0xFF45 - LY Compare : Compares itself with the LY. If the values ar ethe same, set the coincedent flag 

    u16 DMA = 0xFF46; 	//0xFF46 - Direct Memory Access Transfer
    u16 BGP = 0xFF47; 	//0xFF47 - Background Palette Data  0b11 10 01 00

    u8 ie = 0;
    u8 DIV = 0x00;

    Cartridge* cart;
    std::vector<u8> vRam(0xA000 - 0x8000, 0);
    std::vector<u8> iRam(0xC000 - 0xA000, 0);
    std::vector<u8> mbc(0xE000 - 0xC000, 0); // This size should be big enough to cover all RAM banks
    std::vector<u8> OAM(0xFEA0 - 0xFE00, 0);
    std::vector<u8> i2Ram(0xFFFF - 0xFF80, 0); // more internal ram
    std::vector<u8> bootRom = { 0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, 0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04, 0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20, 0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20, 0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50 };
    std::vector<u8> moreRam(0xFF80 - 0xFEA0, 0);


    bool UP = 1, DOWN = 1, LEFT = 1, RIGHT = 1, A = 1, B = 1, START = 1, SELECT = 1, ACTIONS = 1, DIRECTIONS = 1;

    u8 joyPadInputs() {
        u8 foo = 0b11000000;
        u8 acts = A + B * 2 + SELECT * 4 + START * 8 + DIRECTIONS * 16 + ACTIONS * 32;
        u8 dirs = RIGHT + LEFT * 2 + UP * 4 + DOWN * 8 + DIRECTIONS * 16 + ACTIONS * 32;
        if (ACTIONS == 0) {
            return A + B * 2 + SELECT * 4 + START * 8 + DIRECTIONS * 16 + ACTIONS * 32 + foo;
        }

        if (DIRECTIONS == 0) {
            return RIGHT + LEFT * 2 + UP * 4 + DOWN * 8 + DIRECTIONS * 16 + ACTIONS * 32 + foo;
        }
        return 0b11111111;
    }

    u8 read_from_cart(u16 addr) {
        return cart->read(addr);
    }

    u8 readAt(u16 addr) {
        if (addr == 0xFF00) {
            u8 inputs = joyPadInputs();

            return inputs;
        }

        if (addr == 0xFF04) {
            return DIV;
        }

        if (addr < 0x8000) {
            if (addr < 0x0100) {
                if (moreRam[0xFF50 - 0xFEA0] == 0) // if the bootrom is enabled (which it is by default)
                {
                    return bootRom.at(addr);
                }
            }
            return read_from_cart(addr);
        } else if (addr < 0xA000) {
            return vRam.at(addr - 0x8000);
        } else if (addr < 0xC000) {
            return cart->read(addr);
        } else if (addr < 0xE000) {
            return iRam.at(addr - 0xC000);
        } else if (addr < 0xFE00) {
            return iRam.at(addr - 0xE000);
        } else if (addr < 0xFEA0) {
            return OAM.at(addr - 0xFE00);
        } else if (addr < 0xFF80) {
            return moreRam.at(addr - 0xFEA0);
        } else if (0xFF80 <= addr && addr < 0xFFFF) {
            return i2Ram.at(addr - 0xFF80);
        } else if (addr == 0xFFFF) {
            return ie;
        }
        return -1;
    }

    void write(u8 val, u16 addr) {
        if (addr == 0xFF00) {
            DIRECTIONS = ((val >> 4) & 1);
            ACTIONS = ((val >> 5) & 1);

            u8 prevVal = readAt(0xFF00);
            moreRam.at(0xFF00 - 0xFEA0) = (prevVal & 0b11001111) + (val & 0b00110000);
        }

        if (addr == 0xFF46) {
            DMA_routine();
        }

        if (addr == 0xFF04) {
            DIV = 0x00;
        }

        if (addr < 0x8000) {
            cart->write(val, addr);
        } else if (addr < 0xA000) {
            vRam.at(addr - 0x8000) = val;
        } else if (addr < 0xC000) {
            cart->write(val, addr);
        } else if (addr < 0xE000) {
            iRam.at(addr - 0xC000) = val;
        } else if (addr < 0xFE00) {
            iRam.at(addr - 0xE000) = val;
        } else if (addr < 0xFEA0) {
            OAM.at(addr - 0xFE00) = val;
        } else if (addr < 0xFF80) {
            moreRam.at(addr - 0xFEA0) = val;
        } else if (addr < 0xFFFF) {
            i2Ram.at(addr - 0xFF80) = val;
        } else if (addr == 0xFFFF) {
            ie = val;
        }
    }

    void DMA_routine() {
        // the source of the DMA transfer is determined by the value written to the register, starting at XX00 where XX is the value in hex
        // the original gameboy could only take xx to be from 0x00-F1.

        u16 source = (readAt(DMA) << 8);
        u8 data;
        for (int idx = 0; idx < 0xF1; idx++) {

            data = readAt(source + idx);

            // now write the data to OAM
            write(data, 0xFE00 + idx);
        }

        //dump_oam();
    }

    void init_ram(std::string rom_path) {
        cart = cart_factory(readFile(rom_path));
    }

    void dump_oam() {
        u16 addr = 0xFE00;
        u8 byte = 0;
        for (int i = 0; i < 0xF1; i++) {
            byte = readAt(byte + i);
        }
    }

    void d_vram() {
        std::cout << "BGP :" << std::bitset<8>(readAt(BGP)) << std::endl;
        std::cout << "LCDC :" << std::bitset<8>(readAt(LCDC)) << std::endl;

        std::cout << "BG 0x9882 :" << std::hex << unsigned(readAt(0x9882)) << std::endl;

        u16 addr = 0x8B00;
        for (int i = 0; i < 0x0CFF; i += 2) {
            std::cout << std::hex << unsigned(addr + i) << ": " << std::hex << unsigned(readAt(addr + i)) << unsigned(readAt(addr + i + 1)) << std::endl;
        }
        std::cout << std::endl;
    }
}