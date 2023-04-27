#include <iostream>
#include <vector>
#include "render.hpp"

namespace RENDER {
	const int GB_WIDTH = 160;
	const int GB_HEIGHT = 144;

	const int SPRITE_WIDTH = 256;
	const int SPRITE_HEIGHT = 256;

	const int DEBUG_WIDTH = 400;
	const int DEBUG_HEIGHT = 400;

	const int SCREEN_WIDTH = GB_WIDTH + SPRITE_WIDTH;
	const int SCREEN_HEIGHT = SPRITE_HEIGHT;

	const int RENDER_SCALE = 3;

	SDL_Window* Window = NULL;
	SDL_Surface* WindowSurface = NULL;
	SDL_Surface* GbSurface = NULL;
	SDL_Surface* SpriteSurface = NULL;
	SDL_Surface* DebugSurface = NULL;

	SDL_Renderer* debugRenderer = NULL;

	DisplayMode display_mode = GB;

	void setPix(SDL_Surface* surface, int x, int y, int colour) {
		Uint32 alph = 0xFF000000, r = 0x00, g = 0x00, b = 0x00, shade;

		if (colour == 0) {
			alph = 0xFF000000;
			r = 0xFF;
			g = r << 8;
			b = r << 16;
		}
		else if (colour == 1) {
			alph = 0xFF000000;
			r = 0xA0;
			g = r << 8;
			b = r << 16;
		}
		else if (colour == 2) {
			alph = 0xFF000000;
			r = 0x33;
			g = r << 8;
			b = r << 16;
		}
		else if (colour == 3) {
			alph = 0xFF000000;
			r = 0x00;
			g = r << 8;
			b = r << 16;
		}
		else if (colour == 4) {
			alph = 0;
		}
		else {
			std::cout << "Colour code is not valid\n";
			exit(0);
		}

		Uint32 pixelColour = SDL_MapRGBA(WindowSurface->format, r, r, r, alph);

		setPixel(surface, x, y, pixelColour);
	}

	void setGameBoyPixel(int x, int y, int colour) {
		setPix(GbSurface, x, y, colour);
	}

	void setSpriteDisplayPixel(int x, int y, int colour) {
		setPix(SpriteSurface, x, y, colour);
	}

	void setPixel(SDL_Surface* surface, int x, int y, Uint32 Color) {
		SDL_Rect GB_rect = {};
		GB_rect.x = x * RENDER_SCALE;
		GB_rect.y = y * RENDER_SCALE;
		GB_rect.w = RENDER_SCALE;
		GB_rect.h = RENDER_SCALE;

		SDL_FillRect(surface, &GB_rect, Color);
	}

	void setDisplay(DisplayMode mode) {
		if (mode == SPRITE) {
			SDL_SetWindowSize(Window, SPRITE_WIDTH * RENDER_SCALE, SPRITE_HEIGHT * RENDER_SCALE);
			display_mode = SPRITE;
		}
		else if (mode == GB) {
			SDL_SetWindowSize(Window, GB_WIDTH * RENDER_SCALE, GB_HEIGHT * RENDER_SCALE);
			display_mode = GB;
		}
		else if (mode == MEMORY) {
			SDL_SetWindowSize(Window, DEBUG_WIDTH, DEBUG_HEIGHT);
			display_mode = MEMORY;
		}

		WindowSurface = SDL_GetWindowSurface(Window);
	}

	bool init() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			std::cout << "Could not initialise SDL\n" << SDL_GetError();
			return false;
		}

		Window = SDL_CreateWindow("gb emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (GB_WIDTH * RENDER_SCALE), GB_HEIGHT * RENDER_SCALE, SDL_WINDOW_RESIZABLE);

		if (Window == NULL) {
			std::cout << "Window could not be created" << SDL_GetError();
			return false;
		}

		WindowSurface = SDL_GetWindowSurface(Window);
		GbSurface = SDL_CreateRGBSurface(0, GB_WIDTH * RENDER_SCALE, GB_HEIGHT * RENDER_SCALE, 32, 0, 0, 0, 0);
		SpriteSurface = SDL_CreateRGBSurface(0, SPRITE_WIDTH * RENDER_SCALE, SPRITE_HEIGHT * RENDER_SCALE, 32, 0, 0, 0, 0);
		DebugSurface = SDL_CreateRGBSurface(0, DEBUG_WIDTH, DEBUG_HEIGHT, 32, 0, 0, 0, 0);

		debugRenderer = SDL_CreateSoftwareRenderer(DebugSurface);

		SDL_FillRect(WindowSurface, NULL, SDL_MapRGB(WindowSurface->format, 0xA0, 0xBF, 0xA0));
		SDL_UpdateWindowSurface(Window);

		return true;
	}

	void drawFrame() {
		if (display_mode == GB) {
			SDL_Rect GB_rect = {};
			GB_rect.x = 0;
			GB_rect.y = 0;
			GB_rect.w = GB_WIDTH * RENDER_SCALE;
			GB_rect.h = GB_HEIGHT * RENDER_SCALE;
			SDL_BlitSurface(GbSurface, NULL, WindowSurface, &GB_rect);
		}
		else if (display_mode == SPRITE) {
			SDL_Rect SpriteRect = {};
			SpriteRect.x = 0;
			SpriteRect.y = 0;
			SpriteRect.w = SPRITE_WIDTH * RENDER_SCALE;
			SpriteRect.h = SPRITE_HEIGHT * RENDER_SCALE;
			SDL_BlitSurface(SpriteSurface, NULL, WindowSurface, &SpriteRect);
		}
		else if (display_mode == MEMORY) {
			SDL_Rect DebugRect = {};
			DebugRect.x = 0;
			DebugRect.y = 0;
			DebugRect.w = DEBUG_WIDTH;
			DebugRect.h = DEBUG_HEIGHT;
			SDL_BlitSurface(DebugSurface, NULL, WindowSurface, &DebugRect);
			SDL_RenderPresent(debugRenderer);

		}
		SDL_UpdateWindowSurface(Window);
	}

	void clearDebugDisplay() {
		SDL_RenderClear(debugRenderer);
	}

	void drawDebugText(std::string textureText, int x, int y) {}

	void delay(int time) {
		SDL_Delay(time);
	}

	void close() {

		SDL_FreeSurface(WindowSurface);
		WindowSurface = NULL;

		SDL_DestroyWindow(Window);
		Window = NULL;

		SDL_DestroyRenderer(debugRenderer);
		debugRenderer = NULL;

		SDL_Quit();
	}
}
