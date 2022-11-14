#include "game.hpp"

int main(int argc, char* argv[]) {
	std::string rom_path = "C:\\emulator\\n64-roms\\mario.gb";
	game_loop(rom_path);
}