#include <string>
namespace INPUTS {
	extern bool toggle_logging;
	extern bool switch_display;
	void readInputs();
	void waitForInput();
	bool getQuit();
	std::string listen_for_dropped_file();
}