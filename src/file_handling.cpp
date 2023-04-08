#include <string>
#include <vector>
#include <fstream>
#include "typedefs.hpp"

std::vector<u8> readFile(std::string fileName) {
	std::ifstream file(fileName, std::ios::binary);

	std::vector<u8> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>(file));

	return buffer;
}