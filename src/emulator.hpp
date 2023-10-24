#include "run_options.hpp"
#include "input_handler.hpp"
#include "ppu.hpp"

class Emulator {
  RunOptions options;
  InputHandler input_handler;
  PPU* ppu;
public:
  Emulator(RunOptions options);
  void run();
  void game_loop();
  void initialise_state();
  void tick();
  void handle_inputs();
};
