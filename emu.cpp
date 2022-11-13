#include <iostream>
#include <vector>
#include "emu.hpp"

typedef  uint8_t   u8;
typedef   int8_t   s8;
typedef uint16_t  u16;

namespace EMU {
	const int SCREEN_WIDTH = 160;
	const int SCREEN_HEIGHT = 144;

	SDL_Event e;
	SDL_Window* Window = NULL;
	SDL_Surface* Surface = NULL;

	bool quit = false;

	void inputs(){
		while(SDL_PollEvent( &e ) != 0) {
			if(e.type == SDL_QUIT){
            	quit = true;
        	}
    	}
	}

	// Tile_type = 0 for BG, = 1 for window, = 2 for Chars
	void setPix(int x, int y, int colour)
	{
		Uint32 alph = 0xFF000000, r = 0x00, g = 0x00, b = 0x00, shade;


		if (colour == 0)
		{
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
			// r = 0xFF;
		} else if (colour == 3) {
			alph = 0xFF000000;
			r = 0x00;
			g = r << 8;
			b = r << 16;
		} else if (colour == 4) {
			// exit(0);
			alph = 0;
		} else {
			std::cout << "Colour is fucked\n";
			exit(0);
		}


		shade = r + g + b + alph;

		setPixel(Surface, x, y, shade);

	}

	void setPixel(SDL_Surface* surface, int X, int Y, Uint32 Color){
    
		Uint8* pixel = (Uint8*)surface->pixels; // the -> operator gives a pointer to a class variable, and the * before it dereferenecs it
		pixel += (Y * surface->pitch) + (X * sizeof(Uint32));
		*((Uint32*)pixel) = Color;
	}

	bool init() {
	    bool success = true;
		    
			// Initialise SDL
			if (SDL_Init(SDL_INIT_VIDEO) < 0)
			{
				std::cout << "Could not initialise SDL\n" << SDL_GetError();
				success = false;
			}
		    else
		    {
		        // Create window
		        Window = SDL_CreateWindow("gb emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
		        
		        if (Window == NULL)
		        {
					std::cout << "Window could not be created" << SDL_GetError();
		            success = false;
		        }
		        else
				{
					Surface = SDL_GetWindowSurface(Window);
					SDL_FillRect(Surface, NULL, SDL_MapRGB( Surface->format, 0xA0, 0xBF, 0xA0));
					SDL_UpdateWindowSurface(Window);
		        }
		    }
		return success;
	}

	void close(){
	    //Deallocate surface
	    SDL_FreeSurface(Surface);
	    Surface = NULL;
	    
	    //Destroy window
	    SDL_DestroyWindow(Window);
	    Window = NULL;
	    
	    //Quit SDL subsystems
	    SDL_Quit();
	}

	void drawFrame(){
		SDL_UpdateWindowSurface(Window);
	}

	void delay(int time){
		SDL_Delay(time);
	}	
}