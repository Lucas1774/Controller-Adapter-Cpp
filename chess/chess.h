#ifndef CHESS_H
#define CHESS_H

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
    std::unordered_map<int, std::chrono::steady_clock::time_point> pad_to_last_pressed;
    std::unordered_map<int, std::chrono::steady_clock::time_point> pad_to_last_executed;
    std::unordered_map<int, bool> pad_to_is_unleashed;
};
bool updateAbstractState(const int direction, const int &buttonState, State &state, const float res_scaling_x, const float res_scaling_y);
} // namespace chess

#endif // CHESS_H
