#include "sprite_renderer.hpp"
#include "render.hpp"
#include "ram.hpp"
#include "utils.hpp"

SpriteRenderer::SpriteRenderer(RAM* ram) {
    this->ram = ram;
}

void SpriteRenderer::render_sprite(u8 tile_x, u8 tile_y, u8 charcode, u8 palette) {
    int x = tile_x, y = tile_y, colour = 3;
    u8 line1, line2;
    u16 addr = 0x8000 + (charcode << 4);

    for (int i = 0; i < 8; i++) {
        line1 = ram->readAt(addr);
        line2 = ram->readAt(addr + 2);

        addr += 2;

        for (int j = 0; j < 8; j++) {
            colour = ((line1 >> j) & 1) + 2 * ((line2 >> j) & 1);

            if (colour == 0) {
                colour = 4;
            } else if (colour == 1) {
                colour = (palette & 0b00001100) >> 2;
            } else if (colour == 2) {
                colour = (palette & 0b00110000) >> 4;
            } else if (colour == 3) {
                colour = (palette & 0b11000000) >> 6;
            }
            RENDER::setSpriteDisplayPixel(x + j, y + i, colour);
        }
    }
}

void SpriteRenderer::draw_sprite(int spriteNum, int xOffset, int yOffset) {
    u16 spriteSheetStart = 0x8000;

    u16 spriteStart = spriteSheetStart + (spriteNum * 0x10);

    u8 line1, line2;

    for (int y = 0; y < 8; y++) {
        line1 = ram->readAt(spriteStart + y * 2);
        line2 = ram->readAt(spriteStart + y * 2 + 1);

        for (int x = 0; x < 8; x++) {
            int pixelColour = ((line1 >> x) & 1) + 2 * ((line2 >> x) & 1);

            RENDER::setSpriteDisplayPixel(7 - x + xOffset, y + yOffset, pixelColour);
        }
    }
}

void SpriteRenderer::display_sprites() {
    for (int spriteNum = 0; spriteNum < 384; spriteNum++) {
        int xOffset = (spriteNum % 16) * 8;
        int yOffset = (spriteNum / 16) * 8;
        draw_sprite(spriteNum, xOffset, yOffset);
    }
}

void SpriteRenderer::drawObject(int objectNum) {
    u16 objectStartAddr = 0xFE00;
    u16 objectAddr = objectStartAddr + (objectNum * 0x04);

    u8 tileIndex = ram->readAt(objectAddr);

    int xOffset = (objectNum % 16) * 8 + 128;
    int yOffset = (objectNum / 16) * 8;

    draw_sprite(tileIndex, xOffset, yOffset);
}

void SpriteRenderer::displayObjects() {
    for (int objectNum = 0; objectNum < 40; objectNum++) {
        drawObject(objectNum);
    }
}

void SpriteRenderer::displaySpritesFromRAM() {
    int fff = 0;

    u16 addr = 0xFE00;
    u8 palette;

    u8 palette1 = ram->readAt(0xFF48);
    u8 palette2 = ram->readAt(0xFF49);

    for (int i = 0; i < 40; i++) {
        u8 charcode = ram->readAt(addr + 2);

        u8 attrib = ram->readAt(addr + 3);

        if ((attrib & 0b00010000) == 0) {
            palette = palette1;
        } else {
            palette = palette2;
        }

        render_sprite(i % 32, i / 32, charcode, palette);

        addr += 4;
    }
}

