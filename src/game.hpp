#include <iostream>
#include <stdint.h>
#include <vector>
#include <cstring>
#include "typedefs.hpp"

enum Mode {
    NORMAL,
    DEBUG,
};

struct RunOptions {
    bool NO_DISPLAY;
    bool LOG_STATE;
};

void game_loop(std::string rom_path, RunOptions options);

namespace TIMER {
    void inc(int);
    void update();
    void overflow();
} 

namespace LCD {
    void draw_sprites();
    void draw_BG();
    void draw_BGTile(int, u8, u8, u16, u8 palette);
    void render_sprite(u8, u8, u8, u8);
    void draw_Window();
    void update();
    void v_blank();
}
