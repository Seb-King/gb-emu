#pragma once

#include <vector>

using namespace std;

enum Colour {
  LIGHT,
  DARK,
  DARKER,
  DARKEST
};

class PPU {
public:
  virtual vector<vector<Colour>> getBuffer() = 0;
  virtual void update(int ticks) = 0;
};

PPU* buildPPU();
