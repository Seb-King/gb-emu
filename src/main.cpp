#include "emulator.hpp"
#include "run_options.hpp"
#include <functional>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

std::function<void()> loop;
void main_loop() { loop(); }

int main(int argc, char* argv[]) {
	Emulator* emu = new Emulator(parseOptions(argc, argv));

	loop = [&] {
		emu->single_step();
		};

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, true);
#else
	emu->run();
#endif
	while (true) main_loop();
}
