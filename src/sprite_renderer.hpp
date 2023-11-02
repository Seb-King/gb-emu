#pragma once
#include "ram.hpp"

class SpriteRenderer {
public:
  SpriteRenderer(RAM* ram);
  void display_sprites();
  void displayObjects();
  RAM* ram;

private:
  void render_sprite(u8 tile_x, u8 tile_y, u8 charcode, u8 palette);
  void draw_sprite(int spriteNum, int xOffset, int yOffset);
  void displaySpritesFromRAM();
  void drawObject(int objectNum);
};
