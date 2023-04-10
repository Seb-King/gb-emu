#include <string>
#include <vector>
#include <fstream>
#include "typedefs.hpp"
#include <filesystem>
#include <iostream>

std::vector<u8> readFile(std::string fileName) {
	if(!std::filesystem::exists(fileName)) {
		std::cout << "File not found!" << std::endl;
		throw std::runtime_error("File not found!");
	}

	
	std::ifstream file(fileName, std::ios::binary);

	std::vector<u8> buffer((std::istreambuf_iterator<char>(file)), {});

	return buffer;
}