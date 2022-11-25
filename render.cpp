#include <iostream>
#include <vector>
#include "render.hpp"
#include "text.hpp"

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

	Text text;

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

	bool init() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			std::cout << "Could not initialise SDL\n" << SDL_GetError();
			return false;
		} 

		Window = SDL_CreateWindow("gb emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (SCREEN_WIDTH * RENDER_SCALE) + DEBUG_WIDTH, SCREEN_HEIGHT * RENDER_SCALE, SDL_WINDOW_RESIZABLE);

		if (Window == NULL) {
			std::cout << "Window could not be created" << SDL_GetError();
			return false;
		}

		if (TTF_Init() == -1) {
			printf("Could not initialise SDL_TTF");
			return false;
		}

		loadFont();


		WindowSurface = SDL_GetWindowSurface(Window);
		GbSurface = SDL_CreateRGBSurface(0, GB_WIDTH * RENDER_SCALE, GB_HEIGHT * RENDER_SCALE, 32, 0, 0, 0, 0);
		SpriteSurface = SDL_CreateRGBSurface(0, SPRITE_WIDTH * RENDER_SCALE, SPRITE_HEIGHT * RENDER_SCALE, 32, 0, 0, 0, 0);
		DebugSurface = SDL_CreateRGBSurface(0, DEBUG_WIDTH, DEBUG_HEIGHT, 32, 0, 0, 0, 0);

		debugRenderer = SDL_CreateSoftwareRenderer(DebugSurface);

		SDL_FillRect(WindowSurface, NULL, SDL_MapRGB(WindowSurface->format, 0xA0, 0xBF, 0xA0));
		SDL_UpdateWindowSurface(Window);

		loadFont();

		return true;
	}

	void drawFrame() {
		SDL_Rect GB_rect = {};
		GB_rect.x = 0;
		GB_rect.y = 0;
		GB_rect.w = GB_WIDTH * RENDER_SCALE;
		GB_rect.h = GB_HEIGHT * RENDER_SCALE;

		SDL_Rect SpriteRect = {};
		SpriteRect.x = GB_WIDTH * RENDER_SCALE;
		SpriteRect.y = 0;
		SpriteRect.w = SPRITE_WIDTH * RENDER_SCALE;
		SpriteRect.h = SPRITE_HEIGHT * RENDER_SCALE;

		SDL_Rect DebugRect = {};
		DebugRect.x = GB_WIDTH * RENDER_SCALE + SPRITE_WIDTH * RENDER_SCALE;
		DebugRect.y = 0;
		DebugRect.w = DEBUG_WIDTH;
		DebugRect.h = DEBUG_HEIGHT;

		SDL_RenderPresent(debugRenderer);

		SDL_BlitSurface(DebugSurface, NULL, WindowSurface, &DebugRect);
		SDL_BlitSurface(SpriteSurface, NULL, WindowSurface, &SpriteRect);
		SDL_BlitSurface(GbSurface, NULL, WindowSurface, &GB_rect);
		SDL_UpdateWindowSurface(Window);
	}

	void drawText() {
		Text foo;

		SDL_Color black;
		black.r = 255;
		black.g = 255;
		black.b = 255;
		black.a = 255;

		if (!foo.loadFromRenderedText("banana", black, debugRenderer)) {
			printf("Could not load text");
		}

		foo.render(2, 0, debugRenderer);


		if (!foo.loadFromRenderedText("banana", black, debugRenderer)) {
			printf("Could not load text");
		}

		foo.render(20, 80, debugRenderer);


		foo.free();
	}

	void drawDebugText(std::string textureText, int x, int y) {
		Text foo;

		SDL_Color black;
		black.r = 255;
		black.g = 255;
		black.b = 255;
		black.a = 255;

		if (!foo.loadFromRenderedText(textureText, black, debugRenderer)) {
			printf("Could not load text");
		}

		foo.render(x, y, debugRenderer);

		foo.free();
	}

	void delay(int time) {
		SDL_Delay(time);
	}

	void close() {
		closeFont();

	    SDL_FreeSurface(WindowSurface);
		WindowSurface = NULL;
	    
	    SDL_DestroyWindow(Window);
	    Window = NULL;

		SDL_DestroyRenderer(debugRenderer);
		debugRenderer = NULL;
	    
	    SDL_Quit();
	}	
}