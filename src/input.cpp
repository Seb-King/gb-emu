#include <SDL.h>
#include <iostream>
#include "utils.h"

namespace INPUTS {
	bool quit = false;
	bool switch_display = false;
	SDL_Event e;


	void readInputs() {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}

			if (e.type == SDL_KEYDOWN) {
				std::cout << e.type << std::endl;

				if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					quit = true;
				} else if (e.key.keysym.scancode == SDL_SCANCODE_TAB) {
					switch_display = true;
				}
			}
		}
	}

	void waitForInput() {
		SDL_WaitEvent(&e);
	}

	bool getQuit() {
		return quit;
	}
}