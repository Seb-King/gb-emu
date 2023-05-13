#include "emulator.hpp"
#include "run_options.hpp"

int main(int argc, char* argv[]) {
	Emulator* emu = new Emulator(parseOptions(argc, argv));
	emu->run();
}
