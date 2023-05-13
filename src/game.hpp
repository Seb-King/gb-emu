#include <iostream>
#include <stdint.h>
#include <vector>
#include <cstring>
#include "typedefs.hpp"
#include "run_options.hpp"

enum Mode {
    NORMAL,
    DEBUG,
};

void game_loop(RunOptions options);

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
