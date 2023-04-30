#include <SDL.h>
#include <iostream>
#include "utils.hpp"
#include "ram.hpp"
#include "cpu.hpp"

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
				if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					quit = true;
				} else if (e.key.keysym.scancode == SDL_SCANCODE_TAB) {
					switch_display = true;
				} else if (e.key.keysym.scancode == SDL_SCANCODE_Z) {
					RAM::A = 0;
					RAM::B = 0;
					RAM::START = 0;
					RAM::SELECT = 0;
					RAM::DOWN = 0;
				}
			}


			if (e.type == SDL_KEYUP) {
				if (e.key.keysym.scancode == SDL_SCANCODE_Z) {
					CPU::write(RAM::readAt(0xFF0F) | 0b00010000, 0xFF0F);
					RAM::A = 1;
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