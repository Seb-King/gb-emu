#include "game.hpp"

std::string getRomPath() {
	return "/Users/sebking/dev/gb-emu/roms/dr-mario.gb";
}

int main(int argc, char* argv[]) {
	game_loop(getRomPath(), DEBUG);
}
