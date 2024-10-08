#ifndef TFT_H
#define TFT_H
#define NONE -1

#include "funcs.h"
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
};
bool updateAbstractState(const int button, const int &buttonState, State &state, BufferState &bufferState, const float res_scaling_x, const float res_scaling_y, const Functions &functions);
void updateAbstractState(const std::unordered_map<int, int> &buttonState, State &state, const float res_scaling_x, const float res_scaling_y);
} // namespace tft

#endif // TFT_H
