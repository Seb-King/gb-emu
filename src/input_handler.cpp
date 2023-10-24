#include <SDL.h>
#include "input_handler.hpp"
#include "utils.hpp"
#include "ram.hpp"
#include "cpu.hpp"

InputHandler::InputHandler() {}

bool InputHandler::get_quit() {
  return this->quit;
}

bool InputHandler::actions_enabled() {
  u8 val = RAM::readAt(0xFF00);
  return ((val >> 5) & 1) == 0;
}

bool InputHandler::directions_enabled() {
  u8 val = RAM::readAt(0xFF00);
  return ((val >> 4) & 1) == 0;
}

void InputHandler::request_joypad_interrupt() {
  CPU::write(RAM::readAt(0xFF0F) | 0b00010000, 0xFF0F);
}

std::string InputHandler::listen_for_dropped_file() {
  char* dropped_filedir;
  SDL_bool done;
  done = SDL_FALSE;
  while (!done) {
    while (!done && SDL_PollEvent(&e)) {
      switch (e.type) {
      case (SDL_QUIT): {
        done = SDL_TRUE;
        break;
      }

      case (SDL_DROPFILE): {
        done = SDL_TRUE;
        dropped_filedir = e.drop.file;
        break;
      }
      case (SDL_KEYDOWN): {
        if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
          done = SDL_TRUE;
          break;
        }
      }
      }
    }
    SDL_Delay(0);
  }
  return dropped_filedir;
}

void InputHandler::read_and_handle_inputs() {
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
        RAM::SELECT = 0;
        if (this->actions_enabled()) {
          this->request_joypad_interrupt();
        }
      } else if (e.key.keysym.scancode == SDL_SCANCODE_A) {
        RAM::A = 0;

        if (this->actions_enabled()) {
          this->request_joypad_interrupt();
        }
      } else if (e.key.keysym.scancode == SDL_SCANCODE_S) {
        RAM::B = 0;

        if (this->actions_enabled()) {
          this->request_joypad_interrupt();
        }
      } else if (e.key.keysym.scancode == SDL_SCANCODE_X) {
        RAM::START = 0;
        if (this->actions_enabled()) {
          this->request_joypad_interrupt();
        }
      } else if (e.key.keysym.scancode == SDL_SCANCODE_LEFT) {
        RAM::LEFT = 0;

        if (this->directions_enabled()) {
          this->request_joypad_interrupt();
        }
      } else if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
        RAM::RIGHT = 0;

        if (this->directions_enabled()) {
          this->request_joypad_interrupt();
        }
      } else if (e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
        RAM::DOWN = 0;

        if (this->directions_enabled()) {
          this->request_joypad_interrupt();
        }
      } else if (e.key.keysym.scancode == SDL_SCANCODE_UP) {
        RAM::UP = 0;

        if (this->directions_enabled()) {
          this->request_joypad_interrupt();
        }
      } else if (e.key.keysym.scancode == SDL_SCANCODE_L) {
        toggle_logging = !toggle_logging;
      }
    }


    if (e.type == SDL_KEYUP) {
      if (e.key.keysym.scancode == SDL_SCANCODE_Z) {
        RAM::SELECT = 1;
      } else if (e.key.keysym.scancode == SDL_SCANCODE_X) {
        RAM::START = 1;
      } else if (e.key.keysym.scancode == SDL_SCANCODE_A) {
        RAM::A = 1;
      } else if (e.key.keysym.scancode == SDL_SCANCODE_S) {
        RAM::B = 1;
      }
      if (e.key.keysym.scancode == SDL_SCANCODE_LEFT) {
        RAM::LEFT = 1;
      }
      if (e.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
        RAM::RIGHT = 1;
      }
      if (e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
        RAM::DOWN = 1;
      }
      if (e.key.keysym.scancode == SDL_SCANCODE_UP) {
        RAM::UP = 1;
      }
    }
  }
}
