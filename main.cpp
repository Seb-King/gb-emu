#include <SDL2/SDL.h>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <bitset>
#include <iomanip> // Printing variables in hex format
#include "main.hpp"
#include "emu.cpp"
  // Compile using: g++ main.cpp -std=c++11 -lSDL2
  // TODO: Put in function headers for all of the opcodes

  // wrapper to allow two 8 bit registers to be used as a single 16 bit register

u16 reg::val(){ return lo + (hi << 8);}

void reg::set(u16 x) {
    hi = x >> 8;
    lo = (x << 8) >> 8; // this should chop off the most significant 4 bits
}

namespace CPU
{
				   
    u8 IME = 0;
    void STAT();
    void STAT(){

        RAM::write(CPU::PC & 0x0F, CPU::SP); 
        CPU::SP--;
        RAM::write((CPU::PC & 0xF0) >> 8, CPU::SP);
        CPU::PC = 0x0048;

         // now get rid of request flag IF
        RUPS::IF = RAM::readAt(0xFF0F);
        RUPS::IF &= 0b11111101;
        RAM::write(RUPS::IF, 0xFF0F);

    }
    void runOPCode(u8 op_code)
    {
        op_codes[op_code]();

        if (interrupt_disable > 0){
            if(interrupt_disable == 1)
            {
                IME = 0;
                interrupt_disable = 0;
                //std::cout << "Interrupts Disabled. PC: " << std::hex << PC << std::endl << std::endl;
            } else if(interrupt_disable == 2) {
                interrupt_disable--;
            }

            if (interrupt_disable == 3) {
                IME = 1;
                //std::cout << "Interrupts Enabled. PC: " << std::hex << PC << std::endl << std::endl;
                interrupt_disable = 0;
            } else if(interrupt_disable == 4) {
                interrupt_disable--;
            }
        }
    }
}

namespace RAM
{
    std::vector<u8> rom(0x8000, 0); // I have no idea what the size should be
    std::vector<u8> vRam(0xA000 - 0x8000, 0);  
    std::vector<u8> iRam(0xC000 - 0xA000, 0);
    std::vector<u8> mbc (0xE000 - 0xC000, 0); // This size should be big enough to cover all RAM banks
    std::vector<u8> sRam(0xFE00 - 0xE000, 0);
    std::vector<u8> OAM (0xFEA0 - 0xFE00, 0);  // this one needs to be made smaller
    std::vector<u8> i2Ram(0xFFFF - 0xFF80, 0); // more internal ram
    std::vector<u8> bootRom =  {0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, 0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04, 0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20, 0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20, 0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50};
    std::vector<u8> moreRam(0xFF80 - 0xFEA0, 0); //TODO: needs a better name
    u8 readAt(u16 addr)
    {
          // we'll be assuming 32kB cartridge for now and add swapping in later
        if (addr < 0x8000){
            if (addr < 0x0100){
                if (moreRam[0xFF50- 0xFEA0] == 0) // if the bootrom is enabled (which it is by default)
                {
                    return bootRom.at(addr);
                }
            }
            return rom.at(addr);
        }
        else if(addr < 0xA000)                    { return vRam.at(addr - 0x8000); }
        else if(addr < 0xC000)                    { return sRam.at(addr - 0xA000); }
        else if(addr < 0xE000)                    { return iRam.at(addr - 0xC000); }
        else if(addr < 0xFE00)                    { return iRam.at(addr - 0xE000); }
        else if(addr < 0xFEA0)                    { return OAM.at(addr - 0xFE00); }
        else if(addr < 0xFF80)                    { return moreRam.at(addr - 0xFEA0);}
        else if(0xFF80 <= addr && addr < 0xFFFF)  { return i2Ram.at(addr - 0xFF80); }
        else if(addr == 0xFFFF)                   { return ie;}
        return -1;
    }

    void write(u8 val, u16 addr)
    {
        if (addr < 0x8000)                        { std::cout<< "Tried writing to ROM!" << std::hex << unsigned(addr) << " " << std::hex << unsigned(val) << std::endl; }//rom.at(addr) = val; }
        else if(addr < 0xA000)                    { vRam.at(addr - 0x8000) = val; }
        else if(addr < 0xC000)                    { sRam.at(addr - 0xA000) = val; }
        else if(addr < 0xE000)                    { iRam.at(addr - 0xC000) = val; }
        else if(addr < 0xFE00)                    { iRam.at(addr - 0xE000) = val; }
        else if(addr < 0xFEA0)                    { OAM.at(addr - 0xFE00) = val; }
        else if(addr < 0xFF80){ 
            moreRam.at(addr - 0xFEA0) = val; 
            if (addr == DMA) {
                DMA_routine();
            }
        }
        else if(addr < 0xFFFF)  { i2Ram.at(addr - 0xFF80) = val;  } //std::cout << "Writing: " << std::hex <<  val << " to " << std::hex << addr << std::endl << std::endl;
        else if(addr == 0xFFFF)                   { ie = val;}
    }

    //could just make this a function call in CPU
    u8 read()
    { 
        u8 byte = readAt(CPU::PC);
        ++CPU::PC;
        return byte;
    }

    void DMA_routine(){
        // the source of the DMA transfer is determined by the value written to the register, starting at XX00 where XX is the value in hex
        // the original gameboy could only take xx to be from 0x00-F1.

        std::cout << "DMA Time.\n\n";
        
        u16 source = (readAt(DMA) << 8);
       // std::cout << "SOURCE:" << std::hex << unsigned(readAt(0xFF46)) << std::endl;
        u8 data;
        for (int idx = 0; idx < 0xF1; idx++) {

            //std::cout << std::hex << unsigned(source + idx)<< std::endl;
            data = readAt(source + idx);

            // now write the data to OAM
            write(data, 0xFE00 + idx); 
        }
        
        dump_oam();
        std::cout << "DO WE GET HERE";
        // exit(0);
    }

    void init() {
        FILE* rom_file;
        FILE* boot;
        char buffer [100];
        rom_file = fopen("Dr. Mario.gb", "rb");
        //rom_file = fopen("Tetris.gb", "rb");
        boot = fopen("boot.rom", "rb");

        int c;
        int i = 0;
        while ((c = std::fgetc(rom_file)) != EOF) { 
            RAM::rom[i] = c;
            ++i;
        }    

        u8 d = RAM::readAt(0x147);
        // std::cout << std::hex << unsigned(d) << std::endl;
        // exit(0);
    }

    
    void dump_oam(){
        u16 addr = 0xFE00;
        u8 byte;
        for (int i = 0; i < 0xF1; i++) {
            byte = readAt(byte + i);
            std::cout << std::hex << unsigned(addr + i) << ": " << std::hex << unsigned(byte) << std::endl;
        }
    }

//    void dump_mem(int in, int en){
//        u16 addr = in;
//        u8 byte;
//        while(in <= en){
//            byte = readAt(byte + i);
//            std::cout << std::hex << unsigned(addr + i) << ": " << std::hex << unsigned(byte) << std::endl;
//        }
//    }

    void d_vram() {
        // print ram from 8000 to (idk what)
        std::cout << "BGP :" << std::bitset<8>(readAt(BGP)) << std::endl;
        std::cout << "LCDC :" << std::bitset<8>(readAt(LCDC)) << std::endl;
        // BG map
        std::cout << "BG 0x9882 :" << std::hex << unsigned(readAt(0x9882)) << std::endl;

        u16 addr = 0x8B00;
        for (int i = 0; i < 0x0CFF; i += 2) {
            std::cout << std::hex << unsigned(addr + i) << ": " << std::hex << unsigned(readAt(addr + i)) << unsigned(readAt(addr + i + 1)) << std::endl;
        }
        std::cout << std::endl;
    }
}

namespace LCD
{
    void dump_vram();
    void dump_vram(){
    //    u16 addr = 0x8010;
        u16 addr = 0x9000;
        // addr = 0x8010;
        int colour = 0; // colour of the pixel = 0,1,2 probably easiest to write it as a 2 bit binary number
        int tile_num = 0; // change this to character code later
        int x = 8, y = 8;

        s8 point = 0;
        u8 line1, line2;

        // Now we shuffle through each line and find each pixel along the way, let's just do the first tiel to begin with
        // the tile is 8x8, so we have to read 16 bytes (2 for each line) and there are 8 pixels in each horizontal line

        for (tile_num = 0; tile_num < 40; tile_num++){
            for (int j = 0; j < 8; j++){
                y = 7 - j + 8*(tile_num / 20);
                line1 = RAM::readAt(addr);
                line2 = RAM::readAt(addr + 1);
                point += 2;
                addr = 0x9000 + point;

                // now we have to loop through each bit in the lines, extracting the colours and the x and y position
                for (int k = 0; k < 8; k++){
                    colour = 2 * ((line1 >> k) & 0b1) + ((line2 >> k) & 0b1); // can make this a lot shorter but this way preserve readability (for me)

                    x = 7 - k + 8*(tile_num % 20);
                    //std::cout << "Colour " <<  colour << std::endl;
                    EMU::setPix(x, y, colour);
                }
            }
        }
    }

    void update(){
         // -- Display -- //
        u8 IF = RAM::readAt(0xFFFF);
        
        // we need to check that the LCD is turned on
        if ((RAM::readAt(LCDC) >> 7) == 1) {
            LCD::scanline_count -= CPU::cycles;
        }

        if (LCD::scanline_count <= 0) {
            LCD::scanline_count = 456;

            // Increment LY
            u8 l = RAM::readAt(LY);
           RAM::write(l + 1, LY);
            u8 line = RAM::readAt(LY);
        
            if (line == 144) {
                if (flag == 1) {
                    // RAM::d_vram();    
                }
                RAM::write(RAM::readAt(0xFF0F)|0x01, 0xFF0F); // request interrupt

                u8 thing = RAM::readAt(0xFF40);

                u8 LCDC_ = RAM::readAt(LCDC);
                //std::cout << "LCDC:" << std::bitset<8>(LCDC) << std::endl << std::endl;
                if (LCDC_ >> 7 == 1){
                    if ((LCDC_ & 1) == 1){
                        draw_BG();
                        if (((LCDC_ & 0b00100000) >> 5) == 1){
                            draw_Window();
                        }
                    }

                    // Sprites
                    if ((LCDC_  & 2) == 2) {
                        draw_sprites();
                        std::cout << "we got here\n";
                    }
                    
                }

                EMU::drawFrame();
                EMU::delay(5);

            } else if (line > 153) {
                // here we reset LY and reset the flag associated with v-blank
                RAM::write(0, 0xFF44);
                u8 IF = RAM::readAt(0xFF0F); // interrupt flag
                IF &= 0b11111110;
                RAM::write(IF, 0xFF0F);
            }
        }

        // LCDC Status interrupt
        if (CPU::IME == 1 && ((IF & 0x02) >> 1) == 1){
            // if LY = LYC
            if (RAM::readAt(0xFF44) == RAM::readAt(0xFF45)){

                // request interrupt
                RAM::write(RAM::readAt(0xFF0F) | 0b00000010,0xFF0F);
                //CPU::STAT();
                //std::cout << "LY = LYC interrupt" << std::endl;
            }
        }
    }

    void v_blank(){
        std::cout << "V-Blank Time!\n\n";

        u8 STAT = RAM::readAt(0xFF41);
        STAT |= 0b00000001;
        STAT &= 0b11111101;
        RAM::write(STAT, 0xFF41);

        RAM::write(CPU::PC & 0x0F, CPU::SP); 
        CPU::SP--;
        RAM::write((CPU::PC & 0xF0) >> 8, CPU::SP);
        CPU::PC = 0x0040;
        flag = 0;

        // now get rid of request flag IF
        RUPS::IF = RAM::readAt(0xFF0F);
        RUPS::IF &= 0b11111110;
        RAM::write(RUPS::IF, 0xFF0F);

        std::cout << "PC:" << std::hex << unsigned(CPU::PC) << std::endl;
        std::cout << "OPCODOE:" << std::hex << unsigned(RAM::rom[0x40]) << std::endl << std::endl;
    }

    
    void draw_BG(){
        // figure out where the tileset is located for the background
        u8 LCDC_ = RAM::readAt(LCDC);
        u16 addr, tile_addr;

        // TODO: investigate why this only works when ==1 (I thought it should be the other way around)
        // I think it's possible that the BG tilemap always takes the addresses that the window doesn't use
        // because i think that the 

        // this figures where the BG tilemap is 
        if ( ((LCDC_ & 0b00001000) >> 3)  == 0) {
            addr = 0x9800;
        } else {
            addr = 0x9C00;
        }

        //addr = 0x9800;

        if ( ((LCDC_ & 0b00010000) >> 4)  == 0) {
            tile_addr = 0x9000; // tiles are indexed by a signed 8 bit int
        } else {
            tile_addr = 0x8000; // tiles indexed by an unsigned 8-bit integer
        }
        
        u8 charcode;
        u8 scrollX, scrollY;

        scrollY = RAM::readAt(0xFF42);
        scrollX = RAM::readAt(0xFF43);

        u8 palette = RAM::readAt(0xFF47); //BGP

        u16 x;
        for (int j = 0; j < 32*32; j++){
            charcode = RAM::readAt(addr);
            x = tile_addr;

            // TODO: handle signed integers
            if (x == 0x9000) {
                s8 charc = charcode;
                tile_addr = ((0x0900 + charc) << 4); 
            } else {
                tile_addr += (charcode << 4); 
            }
           
            draw_BGTile( j, scrollX, scrollY, tile_addr, palette );

            tile_addr = x;
            addr++;
        }
    }

    // have to check the LCDC to figure out hwo to handle the address of the charcodes
    // TODO: extend this to be a general tile drawing function
    void draw_BGTile( int tile_num, u8 scrollX, u8 scrollY, u16 addr, u8 palette){
        u8 line1, line2;
        int x, y, colour;


        for (int j = 0; j < 8; j++){
            y = (j + 8*(tile_num / 32) - scrollY) % 256;
            line1 = RAM::readAt(addr);
            line2 = RAM::readAt(addr + 1);
            addr += 2;

            // now we have to loop through each bit in the lines, extracting the colours
            for (int k = 0; k < 8; k++){

                //TODO: colour is also determined by the colour palette (same goes for OBJs) at 0xFF47
                colour =  ((line1 >> k) & 1) + 2 *((line2 >> k) & 1); // can make this a lot shorter but this way preserve readability (for me)
                if (colour == 0){
                    colour = palette & 0b00000011;
                } else if (colour == 1){
                    colour = (palette & 0b00001100 ) >> 2;
                } else if (colour == 2){
                    colour = (palette & 0b00110000) >> 4;
                } else if (colour == 3){
                    colour = (palette & 0b11000000) >> 6;
                }
                x = (7 - k + 8*(tile_num % 32) - scrollX) % 256;

                if (x >= 0 && x < 160 && y >= 0 && y < 144) {
                    EMU::setPix(x, y, colour);    
                }
            }
        }
    }

    void draw_Window(){

        // figure out where the tileset is located for the OBJ
        u8  LCDC_ = RAM::readAt(0xFF40);
        u16 addr, tile_addr;
        // address of the tilemap for objects
        
        // we are drawing the window here, so this gives us the WIndow tilemap
        if ( ((LCDC_ & 0b01000000) >> 6)  == 0) {
            addr = 0x9800;
        } else {
            addr = 0x9C00;
        }

        if ( ((LCDC_ & 0b00001000) >> 3)  == 1) {
            tile_addr = 0x9000; // tiles are indexed by a signed 8 bit int
        } else {
            tile_addr = 0x8000; // tiles indexed by an unsigned 8-bit integer
        }
        
        u8 charcode;
        u8 WindowX, WindowY;

        WindowY = RAM::readAt(0xFF4A);
        WindowX = RAM::readAt(0xFF4B) - 7;

        u8 palette = RAM::readAt(0xFF48);

        u16 x;
        for (int j = 0; j < 32*32; j++){
            charcode = RAM::readAt(addr);
            x = tile_addr;

            if (x == 0x9000) {
                s8 s_char = charcode;
                tile_addr = ((0x0900 + s_char) << 4); 
            } else {
                tile_addr += (charcode << 4); 
            }
           
            draw_BGTile( j, WindowX, WindowY, tile_addr, palette);

            tile_addr = x;
            addr++;
        }
    }

    // we can probably merge this and draw_BGTile into the same function with another parameter in the arguments
    
    void render_sprite(u8 tile_x, u8 tile_y, u8 charcode, u8 palette){
        u8 scrollY = RAM::readAt(0xFF42);
        u8 scrollX = RAM::readAt(0xFF43);
        u8 line1, line2;
        u16 addr = 0x8000 + (charcode << 4);
        int x = tile_x, y = tile_y, colour = 3;

        for (int i = 0; i < 8; i++){
            y = (tile_y - 15 + i  - scrollY) % 256;
            line1 = RAM::readAt(addr);
            line2 = RAM::readAt(addr + 2);

            addr += 2;

            for (int j = 0; j < 8; j++){
                colour =  ((line1 >> j) & 1) + 2 *((line2 >> j) & 1);
                x = (-1  - j + tile_x - scrollX) % 256;

                if (x >= 0 && x < 160 && y >= 0 && y < 144) {
                    if (colour == 0){
                        colour = 4;
                    } else if (colour == 1){
                        colour = (palette & 0b00001100 ) >> 2;
                    } else if (colour == 2){
                        colour = (palette & 0b00110000) >> 4;
                    } else if (colour == 3){
                        colour = (palette & 0b11000000) >> 6;
                    }
                    EMU::setPix(x, y, colour);    
                }
            }
        }
    }
    
    void draw_sprites(){
        int fff = 0;

        // The first address in OAM
        u16 addr = 0xFE00;
        u8 palette;

        // x and y define the bottom right corner of the sprite (i think)
        for (int i = 0; i < 40; i++){
            u8 y = RAM::readAt(addr);
            u8 x = RAM::readAt(addr + 1);
            u8 charcode = RAM::readAt(addr + 2);
            std::cout << "addr:" << std::hex << unsigned(addr) << std::endl;
            std::cout << "OBJcode: " << std::hex << unsigned(charcode) << std::endl;
            //exit(0);
            u8 attrib = RAM::readAt(addr + 3);

            if ((attrib & 0b00010000) == 0) {
                palette = RAM::readAt(0xFF48);
            } else {
                palette = RAM::readAt(0xFF49);
            }

            std::cout << "palette:" << std::bitset<8>(palette) << std::endl;
            //exit(0);

            // Sprites are all stored in v-ram from 0x8000-0x8BFF
            // TODO: add in the other parameters for drawing characters, such as the palette and the orientation
            std::cout  << "x: "  << std::hex << unsigned(x) << " y: " << std::hex <<  unsigned(y) << std::endl;
            if (y > 0 && y < 160 && x > 0 && x < 168){
                
                std::cout  << "x: "  << x << " y: " << y << std::endl;
                // exit(0);
                render_sprite(x, y, charcode, palette);
            }
            addr += 4;
        }
    }
}

namespace TIMER {
    void inc(int amount){
        u8 t = RAM::readAt(0xFF05);
        TIMA = t + amount;
        if (TIMA <= t) {
            // Request interrupt
            RAM::write(RAM::readAt(0xFF0F) | 0b00000100, 0xFF0F);
        }
    }

    void update(){
        if (CPU::IME != 0){
            counter += CPU::cycles;
            u8 rate_reg = RAM::readAt(0xFF07);
            int rate = 0;
            if ((rate & 0b00000011) == 0){
                rate = 1024;
            } else if ((rate & 0b00000011) == 1){
                rate = 16;
            } else if ((rate & 0b00000011) == 2){
                rate = 64; 
            } else if ((rate & 0b00000011) == 3){
                rate = 256;
            }
            int amount = counter % rate;
            if (amount != 0) {
                counter = 0;
                TIMER::inc(amount);
            }
        }
    }

    void overflow(){
        RAM::write(CPU::PC & 0x0F, CPU::SP); 
        CPU::SP--;
        RAM::write((CPU::PC & 0xF0) >> 8, CPU::SP);
        CPU::PC = 0x0050;

        // now get rid of request flag IF
        RUPS::IF = RAM::readAt(0xFF0F);
        RUPS::IF &= 0b11111011;
        RAM::write(RUPS::IF, 0xFF0F);
    }
}

namespace RUPS{
    bool handle_interrupts(){
        // check that IME is set to 1
        if ( CPU::IME == 0){
            return false;
        }

        IF = RAM::readAt(0xFF0F);
        IE = RAM::readAt(0xFFFF);

        // Here we need that the interrupts are handled in order of priority, which means that high priority rups are pushed onto the stack last
        // Timer overflow
        // if (((IF & 4) >> 2) == 1 && ((IE & 4) >> 2) == 1){
        //     // timer interrupt
        //     TIMER::overflow();
        // }

        // // LCDC STAT interrupts
        // if (((IF & 2) >> 1) == 1 && ((IE & 2) >> 1) == 1){
        //     CPU::STAT();
        // }

        // CHECK IE and check FLag request for v-blank
        if ((IF & 1) == 1 && (IE & 1) == 1 ){
            LCD::v_blank();
            CPU::halt = false;
            //exit(0);
        }
        return true;
    }
}

int main(){
    std::cout<<std::endl;

    CPU::init_opcodes();
    CPU::init_decoder();
    RAM::init();
    EMU::init();
    LCD::draw_BG();
    EMU::drawFrame();
    EMU::delay(1000);
    
    u16 debug = 0;

    bool rups_handled = false;
    u8 opcode = 0;

    int inp_time = 0;

    // main loop
    while(!EMU::quit) {   
        inp_time++;
        // Handle inputs
        if (inp_time == 100000){
            inp_time = 0;
            for (int i = 0; i < 50; i++){
                EMU::inputs();  
                // RAM::dump_oam();
            }
        }

        if( CPU::PC == 0x00FE){
            std::cout << "--BOOT COMPLETE--\n\n";
            
            LCD::draw_BG();

            EMU::drawFrame();
            
            //EMU::delay(2000);
            //exit(0);
        }
        
        // If no halt, we read and run an instruction
        if (!CPU::halt){
            debug = CPU::PC;
            opcode = RAM::read();
            CPU::runOPCode(opcode);
        }
        
        // prints out opcodes for debugging
    //    if ( flag == 1 || CPU::PC >= 0x100 ){
    //        flag = 1;
    //        std::cout << "PC:" << std::hex << unsigned(debug) << std::endl;
    //        std::cout << "Opcode:" << std::hex << unsigned(opcode) << std::endl << std::endl;
    //    }

        // Interrupts and DMA //
        LCD::update();
        
        // Timer stuff
        TIMER::update();

        // Handle interrupts !!!
        rups_handled = RUPS::handle_interrupts();
    }

    return 0;
}