#include "emulator.hpp"
#include "ram.hpp"
#include "render.hpp"
#include "cpu.hpp"
#include "sprite_renderer.hpp"
#include "interrupts.hpp"

Emulator::Emulator(RunOptions options) {
  this->options = options;
  InputHandler input_handler;
  this->input_handler = input_handler;
  this->input_time = 0;
  bool render_next_vblank = true;
  this->ppu = buildPPU();
}

void Emulator::run() {
  this->game_loop();
}

void Emulator::game_loop() {
  this->initialise_state();

  while (!this->input_handler.get_quit()) {
    this->single_step();
  }
}

void Emulator::single_step() {
  this->input_time++;

  if (this->input_time == 1000) {
    this->input_time = 0;
    handle_inputs();
  }
  tick();

  if (!options.NO_DISPLAY) {
    u8 y_line = RAM::readAt(LY);
    if (y_line == 144 && render_next_vblank) {
      this->render_next_vblank = false;

      RENDER::drawFromPPUBuffer(this->ppu->getBuffer());
      RENDER::drawFrame();
      display_sprites();
      displayObjects();
    }

    if (y_line != 144 && !this->render_next_vblank) {
      this->render_next_vblank = true;
    }
  }
}

void Emulator::initialise_state() {
  if (!this->options.NO_DISPLAY) {
    RENDER::init();

    if (this->options.romPath == "") {
      this->options.romPath = this->input_handler.listen_for_dropped_file();
    }
  }

  CPU::init();
  RAM::init_ram(options.romPath);

  if (!options.NO_DISPLAY) {
    RENDER::drawFrame();
  }

  if (options.SKIP_BOOT) {
    CPU::init_registers_to_skip_boot();
  }
}

void Emulator::tick() {
  if (options.LOG_STATE || this->input_handler.toggle_logging) {
    CPU::print_registers();
  }

  CPU::executeNextOperation();

  ppu->update(CPU::getCyles());
  TIMER::update();
  handle_interrupts();
}
void Emulator::handle_inputs() {
  for (int i = 0; i < 50; i++) {
    this->input_handler.read_and_handle_inputs();
  }

  if (this->input_handler.switch_display) {
    this->input_handler.switch_display = false;
    if (RENDER::display_mode == GB) {
      RENDER::setDisplay(SPRITE);;
    } else if (RENDER::display_mode == SPRITE) {
      RENDER::setDisplay(GB);
    }
  }
}