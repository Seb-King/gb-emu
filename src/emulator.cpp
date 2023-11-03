#include "emulator.hpp"
#include "ram.hpp"
#include "render.hpp"
#include "cpu.hpp"
#include "sprite_renderer.hpp"

Emulator::Emulator(RunOptions options, GB_CPU* cpu) : input_handler(cpu) {
  this->cpu = cpu;
  this->ppu = buildPPU(cpu, &cpu->ram);
  this->options = options;
  this->sprite_renderer = new SpriteRenderer(&cpu->ram);
  this->input_time = 0;
  this->render_next_vblank = true;
}

void Emulator::run() {
  this->game_loop();
}

void Emulator::game_loop() {
  this->initialise_state();

  bool render_next_vblank = true;

  while (!this->input_handler.get_quit()) {
    single_step();
  }
}
void Emulator::initialise_state() {
  if (!this->options.NO_DISPLAY) {
    RENDER::init();

    if (this->options.romPath == "") {
#ifdef __EMSCRIPTEN__
      this->options.romPath = "../web-roms/tetris.gb";
#else
      this->options.romPath = input_handler.listen_for_dropped_file();
#endif
    }
  }

  cpu->ram.init_ram(options.romPath);

  if (!options.NO_DISPLAY) {
    RENDER::drawFrame();
  }

  if (options.SKIP_BOOT) {
    cpu->init_registers_to_skip_boot();
  }
}

void Emulator::tick() {
  if (options.LOG_STATE || this->input_handler.toggle_logging) {
    cpu->print_registers();
  }

  cpu->execute_next_operation();

  ppu->update(cpu->get_cycles());
  cpu->update();
  cpu->handle_interrupts();
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

void Emulator::single_step() {
  if (this->input_handler.get_quit()) {
    std::cout << "returning empty input get quit" << std::endl;
    return;
  }

  input_time++;

  if (input_time == 1000) {
    input_time = 0;
    handle_inputs();
  }
  tick();

  if (!options.NO_DISPLAY) {
    u8 y_line = cpu->ram.readAt(LY);
    if (y_line == 144 && render_next_vblank) {
      render_next_vblank = false;

      RENDER::drawFromPPUBuffer(this->ppu->getBuffer());
      RENDER::drawFrame();
      sprite_renderer->display_sprites();
      sprite_renderer->displayObjects();
    }

    if (y_line != 144 && !render_next_vblank) {
      render_next_vblank = true;
    }
  }
}