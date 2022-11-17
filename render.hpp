#pragma once
#include <SDL.h>

namespace RENDER {
	void setPixel(SDL_Surface* surface, int x, int y, Uint32 Color);
	bool init();
	void close();
	void drawFrame();
	void delay(int time);
	void setGameBoyPixel(int x, int y, int colour);
	void setSpriteDisplayPixel(int x, int y, int colour);
}
