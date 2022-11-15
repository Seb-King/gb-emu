#include <SDL.h>
#include <iostream>
#include "utils.h"

namespace INPUTS {
	bool quit = false;
	SDL_Event e;


	void readInputs() {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
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