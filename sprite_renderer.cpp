#include "sprite_renderer.h"
#include "render.hpp"
#include "ram.hpp"
#include "utils.h"

void render_sprite(u8 tile_x, u8 tile_y, u8 charcode, u8 palette) {
    int x = tile_x, y = tile_y, colour = 3;
    u8 line1, line2;
    u16 addr = 0x8000 + (charcode << 4);

    for (int i = 0; i < 8; i++) {
        line1 = RAM::readAt(addr);
        line2 = RAM::readAt(addr + 2);

        addr += 2;

        for (int j = 0; j < 8; j++) {
            colour = ((line1 >> j) & 1) + 2 * ((line2 >> j) & 1);

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
            RENDER::setSpriteDisplayPixel(x + j, y + i, colour);
        }
    }
}

void draw_sprite(int spriteNum) {
    int xOffset = (spriteNum % 32) * 8;
    int yOffset = (spriteNum / 32) * 8;
    u16 spriteSheetStart = 0x8000;

    u16 spriteStart = spriteSheetStart + (spriteNum * 0x10);


    u8 line1, line2;

    for (int y = 0; y < 8; y++) {
        line1 = RAM::readAt(spriteStart + y * 2);
        line2 = RAM::readAt(spriteStart + y * 2 + 2);

        for (int x = 0; x < 8; x++) {
            int pixelColour = ((line1 >> x) & 1) + 2 * ((line2 >> x) & 1);

            RENDER::setSpriteDisplayPixel(x + xOffset, y + yOffset, pixelColour);
        }
    }
}

void display_sprites() {
    for (int spriteNum = 0; spriteNum < 384; spriteNum++) {
        draw_sprite(spriteNum);
    }
}

void displaySpritesFromRAM() {
    int fff = 0;

    u16 addr = 0xFE00;
    u8 palette;

    u8 palette1 = RAM::readAt(0xFF48);
    u8 palette2 = RAM::readAt(0xFF49);

    for (int i = 0; i < 40; i++) {
        u8 charcode = RAM::readAt(addr + 2);

        u8 attrib = RAM::readAt(addr + 3);

        if ((attrib & 0b00010000) == 0) {
            palette = palette1;
        }
        else {
            palette = palette2;
        }

        render_sprite(i % 32, i / 32, charcode, palette);

        addr += 4;
    }
}

