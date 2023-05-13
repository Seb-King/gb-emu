#include "utils.hpp"

inline void println(string str) {
	std::cout << str << std::endl;
}

bool getBit(u16 val, int bit) {
	return (val >> bit) & 1;
}