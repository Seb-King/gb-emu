#include "game.hpp"

int main(int argc, char* argv[]) {
	RunOptions options;
	options.LOG_STATE = true;
	options.NO_DISPLAY = true;

	std::string rom = "";

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];

		if (arg == "-h" || arg == "--help") {
				std::cout << "Usage: " << argv[0] << " [options]\n";
				std::cout << "Options:\n";
				std::cout << "  -h, --help        Display this help message\n";
				std::cout << "  -r, --rom         Path to rom\n";
				return 0;
		}

		if (arg == "-r" || arg == "--rom") {
				if (i + 1 < argc) {
						rom = argv[i + 1];
						i++;
				} else {
						std::cerr << "Error: missing argument for " << arg << "\n";
						return 1;
				}
		}
	}

	if (rom == "") {
		exit(1);
	}

	game_loop(rom, options);
}
