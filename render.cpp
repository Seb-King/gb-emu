#include <iostream>
#include <vector>
#include "render.hpp"

namespace RENDER {
	const int GB_WIDTH = 160;
	const int GB_HEIGHT = 144;

	const int SPRITE_WIDTH = 256;
	const int SPRITE_HEIGHT = 256;

	const int SCREEN_WIDTH = GB_WIDTH + SPRITE_WIDTH;
	const int SCREEN_HEIGHT = SPRITE_HEIGHT;

	SDL_Window* Window = NULL;
	SDL_Surface* WindowSurface = NULL;
	SDL_Surface* GbSurface = NULL;
	SDL_Surface* SpriteSurface = NULL;

	// Tile_type = 0 for BG, = 1 for window, = 2 for Chars
	void setPix(SDL_Surface* surface, int x, int y, int colour) {
		Uint32 alph = 0xFF000000, r = 0x00, g = 0x00, b = 0x00, shade;

		if (colour == 0) {
			alph = 0xFF000000;
			r = 0xFF;
			g = r << 8;
			b = r << 16;
		} else if (colour == 1) {
			alph = 0xFF000000;
			r = 0xA0;
			g = r << 8;
			b = r << 16;
		} else if (colour == 2) {
			alph = 0xFF000000;
			r = 0x33;
			g = r << 8;
			b = r << 16;
		} else if (colour == 3) {
			alph = 0xFF000000;
			r = 0x00;
			g = r << 8;
			b = r << 16;
		} else if (colour == 4) {
			alph = 0;
		} else {
			std::cout << "Colour code is not valid\n";
			exit(0);
		}

		shade = r + g + b + alph;

		setPixel(surface, x, y, shade);
	}

	void setGameBoyPixel(int x, int y, int colour) {
		setPix(GbSurface, x, y, colour);
	}

	void setSpriteDisplayPixel(int x, int y, int colour) {
		setPix(SpriteSurface, x, y, colour);
	}

	void setPixel(SDL_Surface* surface, int x, int y, Uint32 Color) {
		Uint8* pixel = (Uint8*)surface->pixels;
		pixel += (y * surface->pitch) + (x * sizeof(Uint32));
		*((Uint32*)pixel) = Color;
	}

	bool init() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			std::cout << "Could not initialise SDL\n" << SDL_GetError();
			return false;
		} 

		Window = SDL_CreateWindow("gb emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);

		if (Window == NULL) {
			std::cout << "Window could not be created" << SDL_GetError();
			return false;
		}

		WindowSurface = SDL_GetWindowSurface(Window);
		GbSurface = SDL_CreateRGBSurface(0, GB_WIDTH, GB_HEIGHT, 32, 0, 0, 0, 0);
		SpriteSurface = SDL_CreateRGBSurface(0, SPRITE_WIDTH, SPRITE_HEIGHT, 32, 0, 0, 0, 0);
		SDL_FillRect(WindowSurface, NULL, SDL_MapRGB(WindowSurface->format, 0xA0, 0xBF, 0xA0));
		SDL_UpdateWindowSurface(Window);

		return true;
	}

	void drawFrame() {
		SDL_Rect GB_rect = {};
		GB_rect.x = 0;
		GB_rect.y = 0;
		GB_rect.w = GB_WIDTH;
		GB_rect.h = GB_HEIGHT;

		SDL_Rect SpriteRect = {};
		SpriteRect.x = GB_WIDTH;
		SpriteRect.y = 0;
		SpriteRect.w = SPRITE_WIDTH;
		SpriteRect.h = SPRITE_HEIGHT;

		SDL_BlitSurface(SpriteSurface, NULL, WindowSurface, &SpriteRect);
		SDL_BlitSurface(GbSurface, NULL, WindowSurface, &GB_rect);
		SDL_UpdateWindowSurface(Window);

	}

	void delay(int time) {
		SDL_Delay(time);
	}

	void close() {
	    SDL_FreeSurface(WindowSurface);
		WindowSurface = NULL;
	    
	    SDL_DestroyWindow(Window);
	    Window = NULL;
	    
	    SDL_Quit();
	}	
}