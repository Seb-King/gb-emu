#include "render.hpp"
#include "utils.hpp"
#include "ram.hpp"
#include "cpu.hpp"
#include "sprite_renderer.hpp"
#include "debug_display.hpp"

namespace LCD {
    int scanline_count = 456;

    void dump_vram() {
        //    u16 addr = 0x8010;
        u16 addr = 0x9000;
        // addr = 0x8010;
        int colour = 0; // colour of the pixel = 0,1,2 probably easiest to write it as a 2 bit binary number
        int tile_num = 0; // change this to character code later
        int x = 8, y = 8;

        s8 point = 0;
        u8 line1, line2;

        // Now we shuffle through each line and find each pixel along the way, let's just do the first tile to begin with
        // the tile is 8x8, so we have to read 16 bytes (2 for each line) and there are 8 pixels in each horizontal line

        for (tile_num = 0; tile_num < 40; tile_num++) {
            for (int j = 0; j < 8; j++) {
                y = 7 - j + 8 * (tile_num / 20);
                line1 = RAM::readAt(addr);
                line2 = RAM::readAt(addr + 1);
                point += 2;
                addr = 0x9000 + point;

                // now we have to loop through each bit in the lines, extracting the colours and the x and y position
                for (int k = 0; k < 8; k++) {
                    colour = 2 * ((line1 >> k) & 0b1) + ((line2 >> k) & 0b1); // can make this a lot shorter but this way preserve readability

                    x = 7 - k + 8 * (tile_num % 20);

                    RENDER::setGameBoyPixel(x, y, colour);
                }
            }
        }
    }

    
    void v_blank() {
        u8 STAT = RAM::readAt(0xFF41);
        STAT |= 0b00000001;
        STAT &= 0b11111101;
        CPU::write(STAT, 0xFF41);

        CPU::SP--;
        CPU::write(CPU::PC & 0x0F, CPU::SP);
        CPU::SP--;
        CPU::write((CPU::PC & 0xF0) >> 8, CPU::SP);
        CPU::PC = 0x0040;
        flag = 0;

        RUPS::IF = RAM::readAt(0xFF0F);
        RUPS::IF &= 0b11111110;
        CPU::write(RUPS::IF, 0xFF0F);
    }

    // have to check the LCDC to figure out hwo to handle the address of the charcodes
    // TODO: extend this to be a general tile drawing function
    void draw_BGTile(int tile_num, u8 scrollX, u8 scrollY, u16 addr, u8 palette) {
        u8 line1, line2;
        int x, y, colour;

        for (int j = 0; j < 8; j++) {
            y = (j + 8 * (tile_num / 32) - scrollY) % 256;
            line1 = RAM::readAt(addr);
            line2 = RAM::readAt(addr + 1);
            addr += 2;

            // now we have to loop through each bit in the lines, extracting the colours
            for (int k = 0; k < 8; k++) {

                //TODO: colour is also determined by the colour palette (same goes for OBJs) at 0xFF47
                colour = ((line1 >> k) & 1) + 2 * ((line2 >> k) & 1); // can make this a lot shorter but this way preserve readability (for me)
                if (colour == 0) {
                    colour = palette & 0b00000011;
                }
                else if (colour == 1) {
                    colour = (palette & 0b00001100) >> 2;
                }
                else if (colour == 2) {
                    colour = (palette & 0b00110000) >> 4;
                }
                else if (colour == 3) {
                    colour = (palette & 0b11000000) >> 6;
                }
                x = (7 - k + 8 * (tile_num % 32) - scrollX) % 256;

                if (x >= 0 && x < 160 && y >= 0 && y < 144) {
                    RENDER::setGameBoyPixel(x, y, colour);
                }
            }
        }
    }

    void draw_BG() {
        // figure out where the tileset is located for the background
        u8 LCDC_ = RAM::readAt(LCDC);
        u16 addr, tile_addr;

        // this figures where the BG tilemap is 
        if (((LCDC_ & 0b00001000) >> 3) == 0) {
            addr = 0x9800;
        }
        else {
            addr = 0x9C00;
        }

        //addr = 0x9800;

        if (((LCDC_ & 0b00010000) >> 4) == 0) {
            tile_addr = 0x9000; // tiles are indexed by a signed 8 bit int
        }
        else {
            tile_addr = 0x8000; // tiles indexed by an unsigned 8-bit integer
        }

        u8 charcode;
        u8 scrollX, scrollY;

        scrollY = RAM::readAt(0xFF42);
        scrollX = RAM::readAt(0xFF43);

        u8 palette = RAM::readAt(0xFF47); //BGP

        u16 x;
        for (int j = 0; j < 32 * 32; j++) {
            charcode = RAM::readAt(addr);
            x = tile_addr;

            // TODO: handle signed integers
            if (x == 0x9000) {
                s8 charc = charcode;
                tile_addr = ((0x0900 + charc) << 4);
            }
            else {
                tile_addr += (charcode << 4);
            }

            draw_BGTile(j, scrollX, scrollY, tile_addr, palette);

            tile_addr = x;
            addr++;
        }
    }

    

    void draw_Window() {

        // figure out where the tileset is located for the OBJ
        u8  LCDC_ = RAM::readAt(0xFF40);
        u16 addr, tile_addr;
        // address of the tilemap for objects

        // we are drawing the window here, so this gives us the Window tilemap
        if (((LCDC_ & 0b01000000) >> 6) == 0) {
            addr = 0x9800;
        }
        else {
            addr = 0x9C00;
        }

        if (((LCDC_ & 0b00001000) >> 3) == 1) {
            tile_addr = 0x9000; // tiles are indexed by a signed 8 bit int
        }
        else {
            tile_addr = 0x8000; // tiles indexed by an unsigned 8-bit integer
        }

        u8 charcode;
        u8 WindowX, WindowY;

        WindowY = RAM::readAt(0xFF4A);
        WindowX = RAM::readAt(0xFF4B) - 7;

        u8 palette = RAM::readAt(0xFF48);

        u16 x;
        for (int j = 0; j < 32 * 32; j++) {
            charcode = RAM::readAt(addr);
            x = tile_addr;

            if (x == 0x9000) {
                s8 s_char = charcode;
                tile_addr = ((0x0900 + s_char) << 4);
            }
            else {
                tile_addr += (charcode << 4);
            }

            draw_BGTile(j, WindowX, WindowY, tile_addr, palette);

            tile_addr = x;
            addr++;
        }
    }

    void render_sprite(u8 tile_x, u8 tile_y, u8 charcode, u8 palette) {
        u8 scrollY = RAM::readAt(0xFF42);
        u8 scrollX = RAM::readAt(0xFF43);
        u8 line1, line2;
        u16 addr = 0x8000 + (charcode << 4);
        int x = tile_x, y = tile_y, colour = 3;

        for (int i = 0; i < 8; i++) {
            y = (tile_y - 16 + i - scrollY) % 256;
            line1 = RAM::readAt(addr);
            line2 = RAM::readAt(addr + 1);

            addr += 2;

            for (int j = 0; j < 8; j++) {
                colour = ((line1 >> j) & 1) + 2 * ((line2 >> j) & 1);
                x = (-1 - j + tile_x - scrollX) % 256;

                if (x >= 0 && x < 160 && y >= 0 && y < 144) {
                    if (colour == 0) {
                        colour = 4;
                    }
                    else if (colour == 1) {
                        colour = (palette & 0b00001100) >> 2;
                    }
                    else if (colour == 2) {
                        colour = (palette & 0b00110000) >> 4;
                    }
                    else if (colour == 3) {
                        colour = (palette & 0b11000000) >> 6;
                    }
                    if (colour != 4) {
                        RENDER::setGameBoyPixel(x, y, colour);
                    }
                }
            }
        }
    }

    void draw_sprites() {
        int fff = 0;

        // The first address in OAM
        u16 addr = 0xFE00;
        u8 palette;

        // x and y define the bottom right corner of the sprite (i think)
        for (int i = 0; i < 40; i++) {
            u8 y = RAM::readAt(addr);
            u8 x = RAM::readAt(addr + 1);
            u8 charcode = RAM::readAt(addr + 2);

            u8 attrib = RAM::readAt(addr + 3);

            if ((attrib & 0b00010000) == 0) {
                palette = RAM::readAt(0xFF48);
            } else {
                palette = RAM::readAt(0xFF49);
            }

            // Sprites are all stored in v-ram from 0x8000-0x8BFF
            // std::cout << "x: " << std::hex << unsigned(x) << " y: " << std::hex << unsigned(y) << std::endl;
            if (y > 0 && y < 160 && x > 0 && x < 168) {
                render_sprite(x, y, charcode, palette);
            }
            addr += 4;
        }
    }

    void update() {
        // -- Display -- //
        u8 IF = RAM::readAt(0xFFFF);

        // we need to check that the LCD is turned on
        if ((RAM::readAt(LCDC) >> 7) == 1) {
            LCD::scanline_count -= CPU::getCyles();
        }

        if (LCD::scanline_count <= 0) {
            LCD::scanline_count = 456;

            // Increment LY
            u8 l = RAM::readAt(LY);
            CPU::write(l + 1, LY);
            u8 line = RAM::readAt(LY);

            if (line == 144) {
                if (flag == 1) {
                    // RAM::d_vram();    
                }
                CPU::write(RAM::readAt(0xFF0F) | 0x01, 0xFF0F); // request interrupt

                u8 LCDC_ = RAM::readAt(LCDC);

                if (LCDC_ >> 7 == 1) {
                    if ((LCDC_ & 1) == 1) {
                        draw_BG();
                        if (((LCDC_ & 0b00100000) >> 5) == 1) {
                            draw_Window();
                        }
                    }

                    // Sprites
                    if ((LCDC_ & 2) == 2) {
                        draw_sprites();
                    }

                    display_sprites();
                    displayObjects();
                }

                drawAtStackPointer();
                RENDER::drawFrame();
                RENDER::delay(5);

            }
            else if (line > 153) {
                // here we reset LY and reset the flag associated with v-blank
                CPU::write(0, 0xFF44);
                u8 IF = RAM::readAt(0xFF0F); // interrupt flag
                IF &= 0b11111110;
                CPU::write(IF, 0xFF0F);
            }
        }

        // LCDC Status interrupt
        if (CPU::IME == 1 && ((IF & 0x02) >> 1) == 1) {
            // if LY = LYC
            if (RAM::readAt(0xFF44) == RAM::readAt(0xFF45)) {

                // request interrupt
                CPU::write(RAM::readAt(0xFF0F) | 0b00000010, 0xFF0F);
                //CPU::STAT();
                //std::cout << "LY = LYC interrupt" << std::endl;
            }
        }
    }
}

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
        u8 TAC = RAM::readAt(0xFF07);
        if (((TAC & 0b00000100) == 0b00000100)) {
            counter += CPU::cycles;
            int rate = 0;
            if ((TAC & 0b00000011) == 0) {
                rate = 1024;
            }
            else if ((TAC & 0b00000011) == 1) {
                rate = 16;
            }
            else if ((TAC & 0b00000011) == 2) {
                rate = 64;
            }
            else if ((TAC & 0b00000011) == 3) {
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
    CPU::SP--;
    CPU::write((CPU::PC & 0xFF00) >> 8, CPU::SP);
    CPU::SP--;
    CPU::write(CPU::PC & 0x00FF, CPU::SP);
    CPU::PC = 0x0060;

    RUPS::IF = RAM::readAt(0xFF0F);
    RUPS::IF &= 0b11101111;
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

        LCD::v_blank();

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