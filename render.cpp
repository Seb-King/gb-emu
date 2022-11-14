#include <iostream>
#include <vector>
#include "render.hpp"

namespace RENDER {
	const int GB_WIDTH = 160;
	const int GB_HEIGHT = 144;

	const int SCREEN_WIDTH = GB_WIDTH * 2;
	const int SCREEN_HEIGHT = 144;

	SDL_Event e;
	SDL_Window* Window = NULL;
	SDL_Surface* WindowSurface = NULL;
	SDL_Surface* GbSurface = NULL;


	bool quit = false;

	void inputs(){
		while(SDL_PollEvent( &e ) != 0) {
			if(e.type == SDL_QUIT){
            	quit = true;
        	}
    	}
	}

	bool getQuit() {
		return quit;
	}

	// Tile_type = 0 for BG, = 1 for window, = 2 for Chars
	void setPix(int x, int y, int colour) {
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

		setPixel(GbSurface, x, y, shade);
	}

	void setPixel(SDL_Surface* surface, int X, int Y, Uint32 Color) {
		Uint8* pixel = (Uint8*)surface->pixels;
		pixel += (Y * surface->pitch) + (X * sizeof(Uint32));
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
		SDL_FillRect(WindowSurface, NULL, SDL_MapRGB(WindowSurface->format, 0xA0, 0xBF, 0xA0));
		SDL_UpdateWindowSurface(Window);

		return true;
	}

	void close() {
	    //Deallocate surface
	    SDL_FreeSurface(WindowSurface);
		WindowSurface = NULL;
	    
	    //Destroy window
	    SDL_DestroyWindow(Window);
	    Window = NULL;
	    
	    //Quit SDL subsystems
	    SDL_Quit();
	}

	void drawFrame() {
		SDL_Rect rect = {};
		rect.x = 0;
		rect.y = 0;
		rect.w = GB_WIDTH;
		rect.h = GB_HEIGHT;

		SDL_BlitSurface(GbSurface, NULL, WindowSurface, &rect);
		SDL_UpdateWindowSurface(Window);
	}

	void delay(int time){
		SDL_Delay(time);
	}	
}