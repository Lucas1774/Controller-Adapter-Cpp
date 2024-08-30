#ifndef TFT_H
#define TFT_H
#define NONE -1

#include "constants.h"
#include <SDL2/SDL.h>
#include <json/json.h>
#include <unordered_map>
#include <chrono>

namespace tft {
void run(
    std::unordered_map<int, int> &buttonState,
    bool hasTriggers,
    Json::Value &config,
    int screenWidth,
    int screenHeight,
    SDL_Joystick *joystick);
enum MouseMovementWithPadMode {
    BOARD,
    ITEMS,
    SHOP,
    CARDS,
    FREE,
};
struct State {
    int boardRow;
    int boardColumn;
    int itemIndex;
    int shopIndex;
    int cardRow;
    int cardColumn;
    MouseMovementWithPadMode mode;
    std::pair<int, int> mouse_target;
    std::unordered_map<int, std::chrono::steady_clock::time_point> pad_to_last_pressed;
    std::unordered_map<int, std::chrono::steady_clock::time_point> pad_to_last_executed;
    std::unordered_map<int, bool> pad_to_is_unleashed;
};
void updateAbstractState(const Direction direction, int buttonState, State &state);
} // namespace tft

#endif // TFT_H
