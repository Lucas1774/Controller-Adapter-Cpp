#ifndef SWARM_H
#define SWARM_H

#include <SDL2/SDL.h>
#include <json/json.h>
#include <unordered_map>

namespace swarm {
void run(std::unordered_map<int, int> &buttonState,
    const bool &hasTriggers,
    const Json::Value &config,
    const int screenWidth,
    const int screenHeight,
    SDL_Joystick *joystick);
}

#endif // SWARM_H
