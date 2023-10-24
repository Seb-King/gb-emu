#include <iostream>

class InputHandler {
public:
  InputHandler();
  bool get_quit();
  std::string listen_for_dropped_file();
  void read_and_handle_inputs();

  bool switch_display = false;
  bool toggle_logging = false;

private:
  bool actions_enabled();
  bool directions_enabled();
  void request_joypad_interrupt();

  bool quit = false;
  SDL_Event e;
};
