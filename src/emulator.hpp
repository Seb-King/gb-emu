#pragma once
#include "run_options.hpp"
#include "input_handler.hpp"
#include "ppu.hpp"
#include "cpu.hpp"
#include "sprite_renderer.hpp"

class Emulator {
  RunOptions options;
  InputHandler input_handler;
  PPU* ppu;
  GB_CPU* cpu;
  SpriteRenderer* sprite_renderer;
  int input_time;
  bool render_next_vblank;
public:
  Emulator(RunOptions options, GB_CPU* cpu);
  void run();
  void game_loop();
  void initialise_state();
  void tick();
  void handle_inputs();
  void single_step();
};
