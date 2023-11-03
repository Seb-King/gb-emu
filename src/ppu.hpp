#pragma once

#include <vector>
#include "cpu.hpp"

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
  GB_CPU* cpu;
  RAM* ram;
};

PPU* buildPPU(GB_CPU* cpu, RAM* ram);
