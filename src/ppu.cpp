#include <vector>
#include "typedefs.hpp"
#include "ram.hpp"
#include "utils.hpp"
#include "ram.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

using namespace std;

enum ObjSize {
  BIG,
  SMALL
};

Colour toColour(int val) {
  switch (val) {
  case (3): {
    return LIGHT;
  }
  case (2): {
    return DARK;
  }
  case (1): {
    return DARKER;
  }
  case (0): {
    return DARKEST;
  }
  }

  return LIGHT;
}

class DmgPPU : public PPU {
  int scanline_count = 456;
  vector<vector<Colour>> buffer = vector<vector<Colour>>(160, vector<Colour>(144));

  void drawToBuffer(int x, int y, int colour) {
    this->buffer.at(x).at(y) = toColour(colour);
  }

  void draw_BGTile(int tile_num, u8 scrollX, u8 scrollY, u16 addr, u8 palette) {
    u8 line1, line2;
    int x, y, colour;

    for (int j = 0; j < 8; j++) {
      y = (j + 8 * (tile_num / 32) - scrollY) % 256;
      line1 = RAM::readAt(addr);
      line2 = RAM::readAt(addr + 1);
      addr += 2;

      for (int k = 0; k < 8; k++) {
        colour = ((line1 >> k) & 1) + 2 * ((line2 >> k) & 1);
        if (colour == 0) {
          colour = palette & 0b00000011;
        } else if (colour == 1) {
          colour = (palette & 0b00001100) >> 2;
        } else if (colour == 2) {
          colour = (palette & 0b00110000) >> 4;
        } else if (colour == 3) {
          colour = (palette & 0b11000000) >> 6;
        }
        x = (7 - k + 8 * (tile_num % 32) - scrollX) % 256;

        if (x >= 0 && x < 160 && y >= 0 && y < 144) {
          drawToBuffer(x, y, colour);
        }
      }
    }
  }

  void draw_BG() {
    u8 LCDC_ = RAM::readAt(LCDC);
    u16 addr, tile_addr;

    if (((LCDC_ & 0b00001000) >> 3) == 0) {
      addr = 0x9800;
    } else {
      addr = 0x9C00;
    }

    if (((LCDC_ & 0b00010000) >> 4) == 0) {
      tile_addr = 0x9000;
    } else {
      tile_addr = 0x8000;
    }

    u8 charcode;
    u8 scrollX, scrollY;

    scrollY = RAM::readAt(0xFF42);
    scrollX = RAM::readAt(0xFF43);

    u8 palette = RAM::readAt(0xFF47);

    u16 x;
    for (int j = 0; j < 32 * 32; j++) {
      charcode = RAM::readAt(addr);
      x = tile_addr;

      if (x == 0x9000) {
        s8 charc = charcode;
        tile_addr = ((0x0900 + charc) << 4);
      } else {
        tile_addr += (charcode << 4);
      }

      draw_BGTile(j, scrollX, scrollY, tile_addr, palette);

      tile_addr = x;
      addr++;
    }
  }


  void draw_Window() {
    u8  LCDC_ = RAM::readAt(0xFF40);
    u16 addr, tile_addr;

    if (((LCDC_ & 0b01000000) >> 6) == 0) {
      addr = 0x9800;
    } else {
      addr = 0x9C00;
    }

    if (((LCDC_ & 0b00001000) >> 3) == 1) {
      tile_addr = 0x9000;
    } else {
      tile_addr = 0x8000;
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
      } else {
        tile_addr += (charcode << 4);
      }

      draw_BGTile(j, WindowX, WindowY, tile_addr, palette);

      tile_addr = x;
      addr++;
    }
  }

  ObjSize getSpriteSize() {
    u8 LCDC_ = RAM::readAt(0xFF40);
    if ((LCDC_ >> 2) & 1) {
      return BIG;
    } else {
      return SMALL;
    }
  }

  void render_sprite(u8 tile_x, u8 tile_y, u8 charcode, u8 palette, u8 attrib) {
    u8 line1, line2;
    u16 addr = 0x8000 + (charcode << 4);
    int x = tile_x, y = tile_y, colour = 3;

    bool xFlip = getBit(attrib, 5);
    bool yFlip = getBit(attrib, 6);
    bool behindBG = getBit(attrib, 7);

    ObjSize size = getSpriteSize();
    int spriteHeight = 8;
    if (size == BIG) {
      spriteHeight = 16;
    }

    for (int i = 0; i < spriteHeight; i++) {
      if (yFlip) {
        if (spriteHeight == BIG) {
          y = (tile_y - i) % 256;
        } else {
          y = (tile_y - 8 - i) % 256;
        }
      } else {
        y = (tile_y - 16 + i) % 256;
      }

      line1 = RAM::readAt(addr);
      line2 = RAM::readAt(addr + 1);

      addr += 2;

      for (int j = 0; j < 8; j++) {
        colour = ((line1 >> j) & 1) + 2 * ((line2 >> j) & 1);

        if (xFlip) {
          x = (-1 + j + tile_x - 8) % 256;
        } else {
          x = (-1 - j + tile_x) % 256;
        }

        if (x >= 0 && x < 160 && y >= 0 && y < 144) {
          if (colour == 0) {
            colour = 4;
          } else if (colour == 1) {
            colour = (palette & 0b00001100) >> 2;
          } else if (colour == 2) {
            colour = (palette & 0b00110000) >> 4;
          } else if (colour == 3) {
            colour = (palette & 0b11000000) >> 6;
          }
          if (colour != 4) {
            drawToBuffer(x, y, colour);
          }
        }
      }
    }
  }

  void draw_sprites() {
    u16 addr = 0xFE00;
    u8 palette;

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

      if (y > 0 && y < 160 && x > 0 && x < 168) {
        render_sprite(x, y, charcode, palette, attrib);
      }
      addr += 4;
    }
  }

  void set_mode_to(u8 mode) {
    u8 stat = RAM::readAt(0xFF41);
    stat = (stat & 0b11111100) + mode;
    RAM::write(stat, 0xFF41);
  }

public:
  vector<vector<Colour>> getBuffer() {
    return this->buffer;
  }

  void update(int ticks) {
    u8 IF = RAM::readAt(0xFFFF);

    if ((RAM::readAt(LCDC) >> 7) == 1) {
      scanline_count -= ticks;
    }

    u8 l = RAM::readAt(LY);

    if (l < 144) {
      if (scanline_count < 456) {
        set_mode_to(2);
      }
      if (scanline_count < 200) {
        set_mode_to(3);
      }
      if (scanline_count <= 0) {
        set_mode_to(0);
      }
    } else {
      set_mode_to(1);
    }

    if (scanline_count <= 0) {
      scanline_count = 700;
      set_mode_to(0);

      CPU::write(l + 1, LY);
      u8 line = RAM::readAt(LY);

      if (line == 144) {
        CPU::write(RAM::readAt(0xFF0F) | 0x01, 0xFF0F);

        u8 LCDC_ = RAM::readAt(LCDC);

        if (getBit(LCDC_, 7)) {
          if (getBit(LCDC_, 0)) {
            draw_BG();
            if (getBit(LCDC_, 5)) {
              draw_Window();
            }
          }

          if ((LCDC_ & 2) == 2) {
            draw_sprites();
          }
        }
      } else if (line > 153) {
        CPU::write(0, 0xFF44);
      }
    }

    if (CPU::IME == 1 && ((IF & 0x02) >> 1) == 1) {
      if (RAM::readAt(0xFF44) == RAM::readAt(0xFF45)) {
        CPU::write(RAM::readAt(0xFF0F) | 0b00000010, 0xFF0F);
      }
    }
  }
};

PPU* buildPPU() {
  return new DmgPPU();
}
