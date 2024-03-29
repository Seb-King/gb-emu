#include <SDL.h>
#include <string>
#include "ppu.hpp"

enum DisplayMode {
	GB,
	SPRITE,
	MEMORY,
};

namespace RENDER {
	extern DisplayMode display_mode;
	void setPixel(SDL_Surface* surface, int x, int y, Uint32 Color);
	bool init();
	void close();
	void drawFrame();
	void delay(int time);
	void setGameBoyPixel(int x, int y, int colour);
	void setSpriteDisplayPixel(int x, int y, int colour);
	void drawText();
	void drawDebugText(std::string textureText, int x, int y);
	void clearDebugDisplay();
	void setDisplay(DisplayMode mode);
	void drawFromPPUBuffer(vector<vector<Colour>> pixelMap);
}
