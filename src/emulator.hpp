#include "run_options.hpp"
#include "input_handler.hpp"
#include "ppu.hpp"
#include "cpu.hpp"

class Emulator {
  RunOptions options;
  InputHandler input_handler;
  PPU* ppu;
  GB_CPU* cpu;
public:
  Emulator(RunOptions options, GB_CPU* cpu);
  void run();
  void game_loop();
  void initialise_state();
  void tick();
  void handle_inputs();
};
