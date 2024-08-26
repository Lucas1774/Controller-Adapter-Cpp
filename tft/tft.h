#ifndef TFT_H
#define TFT_H

#include <SDL2/SDL.h>
#include <json/json.h>
#include <unordered_map>
#include <string>

namespace tft
{
    void run(
        std::unordered_map<std::string, std::string> &buttonState,
        bool hasTriggers,
        Json::Value &config,
        int screenWidth,
        int screenHeight,
        SDL_Joystick *joystick);
}

#endif // TFT_H
