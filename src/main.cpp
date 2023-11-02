#include "emulator.hpp"
#include "run_options.hpp"
#include "cpu.hpp"
#include <functional>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

std::function<void()> loop;
void main_loop() { loop(); }

int main(int argc, char* argv[]) {
	RAM ram;
	GB_CPU cpu(ram);
	Emulator* emu = new Emulator(parseOptions(argc, argv), &cpu);
	emu->run();

	loop = [&] {
		emu->single_step();
		};

	emu->run();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, true);
#else
#endif
	// while (true) main_loop();
}
