#include "debug_display.hpp"
#include "render.hpp"
#include "ram.hpp"
#include "cpu.h"
#include <sstream>

void drawAtStackPointer() {
	std::stringstream stream;


	RENDER::clearDebugDisplay();
	u8 byte = RAM::readAt(CPU::PC);

	stream << "STACK " << std::hex << byte;

	RENDER::drawDebugText(stream.str(), 20, 10);
}
