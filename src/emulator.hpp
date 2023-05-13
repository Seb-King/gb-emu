#include "game.hpp"
#include "run_options.hpp"

class Emulator {
  RunOptions options;
public:
  Emulator(RunOptions options);
  void run();
};
