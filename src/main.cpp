#include "game.hpp"

std::string getRomPath() {
	return "/Users/sebking/dev/gb-emu/roms/dr-mario.gb"; 
}

int main(int argc, char* argv[]) {
	RunOptions options;
	options.LOG_STATE = false;
	options.NO_DISPLAY = false;
	game_loop(getRomPath(), options);
}
