#ifndef TFT_H
#define TFT_H
#define NONE -1

#include <SDL2/SDL.h>
#include <json/json.h>
#include <string>
#include <unordered_map>

namespace tft {
void run(
    std::unordered_map<std::string, std::string> &buttonState,
    bool hasTriggers,
    Json::Value &config,
    int screenWidth,
    int screenHeight,
    SDL_Joystick *joystick);
enum MouseMovementWithPadMode {
    BOARD,
    ITEMS,
    SHOP,
    AUGMENT,
};
enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};
struct State {
    int boardRow;
    int boardColumn;
    int shopIndex;
    std::pair<int, int> mouse_target;
};
extern const std::vector<std::vector<std::pair<int, int>>> BOARD_COORDINATES;
void updateAbstractState(MouseMovementWithPadMode mode, tft::Direction direction, State &state);
} // namespace tft

#endif // TFT_H
