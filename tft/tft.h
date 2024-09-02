#ifndef TFT_H
#define TFT_H
#define NONE -1

#include <SDL2/SDL.h>
#include <chrono>
#include <json/json.h>
#include <unordered_map>

namespace tft {
void run(std::unordered_map<int, int> &buttonState,
         const bool &hasTriggers,
         const Json::Value &config,
         const int screenWidth,
         const int screenHeight,
         SDL_Joystick *joystick);
enum MouseMovementWithPadMode {
    BOARD,
    ITEMS,
    SHOP,
    CARDS,
    LOCK,
    FREE,
};
struct State {
    int boardRow;
    int boardColumn;
    int itemIndex;
    int shopIndex;
    int cardRow;
    int cardColumn;
    int lockIndex;
    MouseMovementWithPadMode mode;
    MouseMovementWithPadMode previous_mode;
    std::pair<int, int> mouse_target;
    std::unordered_map<int, std::chrono::steady_clock::time_point> pad_to_last_pressed;
    std::unordered_map<int, std::chrono::steady_clock::time_point> pad_to_last_executed;
    std::unordered_map<int, bool> pad_to_is_unleashed;
};
bool updateAbstractState(const int direction, const int &buttonState, State &state, const float res_scaling_x, const float res_scaling_y);
bool updateAbstractState(const std::unordered_map<int, int> &buttonState, State &state, const float res_scaling_x, const float res_scaling_y);
} // namespace tft

#endif // TFT_H
