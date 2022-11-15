#include "game.hpp"

std::string getRomPath() {
	return "C:\\emulator\\n64-roms\\mario.gb";
}

int main(int argc, char* argv[]) {
	std::string rom_path = getRomPath();

	game_loop(rom_path, DEBUG);
}
