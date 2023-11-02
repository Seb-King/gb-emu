#include "emulator.hpp"
#include "run_options.hpp"
#include "cpu.hpp"

int main(int argc, char* argv[]) {
	RAM ram;
	GB_CPU cpu(ram);
	Emulator* emu = new Emulator(parseOptions(argc, argv), &cpu);
	emu->run();
}
