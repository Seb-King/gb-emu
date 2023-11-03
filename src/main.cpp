#include "emulator.hpp"
#include "run_options.hpp"
#include "cpu.hpp"

#ifdef __EMSCRIPTEN__
#include <functional>
#include <emscripten.h>
#endif

std::function<void()> loop;
void main_loop() {
	loop();
}

int main(int argc, char* argv[]) {
	RAM ram;
	GB_CPU cpu(ram);
	Emulator* emu = new Emulator(parseOptions(argc, argv), &cpu);

#ifdef __EMSCRIPTEN__
	emu->initialise_state();

	loop = [&] {
		for (int i = 0; i < 10000; i++) {
			emu->single_step();
		}
		};
	emscripten_set_main_loop(main_loop, 0, true);
#else
	emu->run();
#endif
}
