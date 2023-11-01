#include "run_options.hpp"
#include "input_handler.hpp"
#include "ppu.hpp"

class Emulator {
  RunOptions options;
  InputHandler input_handler;
  PPU* ppu;
  int input_time;
  bool render_next_vblank;
public:
  Emulator(RunOptions options);
  void run();
  void game_loop();
  void single_step();
  void initialise_state();
  void tick();
  void handle_inputs();
};
