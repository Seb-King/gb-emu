#pragma once
#include <SDL.h>

namespace EMU {
	bool getQuit();
	void setPixel(SDL_Surface*, int, int, Uint32);
	bool init();
	void close();
	void drawFrame();
	void setPix(int, int, int); // x, y, colour = 0,1,2 or 3 in order from lightest to darkest (or reversed idk)
	void delay(int time);
	void inputs();
}
