#ifndef CHESS_H
#define CHESS_H

#include "funcs.h"
#include <SDL2/SDL.h>
#include <chrono>
#include <json/json.h>
#include <unordered_map>

namespace chess {
void run(std::unordered_map<int, int> &buttonState,
         const bool &hasTriggers,
         const Json::Value &config,
         const int screenWidth,
         const int screenHeight,
         SDL_Joystick *joystick);
enum Mode {
    BOARD,
    RESIGN,
    DRAW,
};
struct State {
    int boardRow;
    int boardColumn;
    int resignIndex;
    int drawIndex;
    Mode mode;
    std::pair<int, int> mouse_target;
};
bool updateAbstractState(const int button, const int &buttonState, State &state, BufferState &BufferState, float res_scaling_x, const float res_scaling_y, const Functions &functions);
} // namespace chess

#endif // CHESS_H
