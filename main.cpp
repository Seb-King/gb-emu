#include "game.hpp"

std::string getRomPath() {
	return "C:\\emulator\\n64-roms\\mario.gb";
}

int main(int argc, char* argv[]) {
	game_loop(getRomPath(), DEBUG);
}
