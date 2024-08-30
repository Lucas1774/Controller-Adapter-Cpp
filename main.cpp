#define SDL_MAIN_HANDLED

#include "constants.h"
#include "swarm.h"
#include "tft.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <windows.h>

using namespace swarm;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "No game was specified" << std::endl;
        return 1;
    }
    std::string game = argv[1];

    Json::Value config;
    std::ifstream configFile("config.json", std::ifstream::binary);
    if (configFile.is_open()) {
        configFile >> config;
        configFile.close();
    } else {
        std::cerr << "Error opening config file." << std::endl;
        return 1;
    }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        std::cerr << "SDL initialization error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) != 0) {
        std::cerr << "SDL display mode error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    int screenWidth = dm.w;
    int screenHeight = dm.h;

    SDL_Joystick *joystick = nullptr;
    bool hasTriggers = false;
    if (SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0);
        SDL_PollEvent(nullptr);
        hasTriggers = SDL_JoystickGetAxis(joystick, config["left_trigger_id"].asInt()) != 0;
    } else {
        std::cerr << "No controller found" << std::endl;
        SDL_Quit();
        return 1;
    }

    std::unordered_map<int, int> buttonState = {
        {A, RELEASED}, {B, RELEASED}, {X, RELEASED}, {Y, RELEASED}, {L1, RELEASED}, {R1, RELEASED}, {SELECT, RELEASED}, {START, RELEASED}, {L3, RELEASED}, {R3, RELEASED}, {XBOX, RELEASED}, {PAD_UP, RELEASED}, {PAD_LEFT, RELEASED}, {PAD_RIGHT, RELEASED}, {PAD_DOWN, RELEASED}, {L2, RELEASED}, {R2, RELEASED}, {ACTIVATE, RELEASED}, {LEFT_JS_LEFT, RELEASED}, {LEFT_JS_RIGHT, RELEASED}, {LEFT_JS_UP, RELEASED}, {LEFT_JS_DOWN, RELEASED}, {RIGHT_JS_LEFT, RELEASED}, {RIGHT_JS_RIGHT, RELEASED}, {RIGHT_JS_UP, RELEASED}, {RIGHT_JS_DOWN, RELEASED}};

    if (game == "swarm") {
        swarm::run(buttonState, hasTriggers, config, screenWidth, screenHeight, joystick);
    } else if (game == "tft") {
        tft::run(buttonState, hasTriggers, config, screenWidth, screenHeight, joystick);
    } else {
        std::cerr << "Invalid game" << std::endl;
    }

    if (joystick) {
        SDL_JoystickClose(joystick);
    }
    SDL_Quit();

    return 0;
}
