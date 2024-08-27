#ifndef SWARM_H
#define SWARM_H

#include <SDL2/SDL.h>
#include <json/json.h>
#include <string>
#include <unordered_map>

namespace swarm {
void run(
    std::unordered_map<std::string, std::string> &buttonState,
    bool hasTriggers,
    Json::Value &config,
    int screenWidth,
    int screenHeight,
    SDL_Joystick *joystick);
}

#endif // SWARM_H
