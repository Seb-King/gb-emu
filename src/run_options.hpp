#pragma once

#include <string>

using namespace std;

struct RunOptions {
  bool NO_DISPLAY;
  bool LOG_STATE;
  bool SKIP_BOOT;
  string romPath;
};

RunOptions parseOptions(int argc, char* argv[]);
