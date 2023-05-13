#include "emulator.hpp"

Emulator::Emulator(RunOptions options) {
  this->options = options;
}

void Emulator::run() {
  game_loop(options);
}