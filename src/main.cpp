#include "emulator.hpp"
#include "run_options.hpp"
#include "cpu.hpp"

int main(int argc, char* argv[]) {
	GB_CPU cpu;
	Emulator* emu = new Emulator(parseOptions(argc, argv), cpu);
	emu->run();
}
