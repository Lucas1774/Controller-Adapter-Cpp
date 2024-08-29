#include "tft.h"
#include "funcs.h"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace tft {
const std::vector<std::vector<std::pair<int, int>>> BOARD_COORDINATES = {
    {{10, 1070}, {248, 1070}, {486, 1070}, {724, 1070}, {962, 1070}, {1200, 1070}, {1438, 1070}, {1676, 1070}, {1914, 1070}},
    {{10, 805}, {248, 805}, {486, 805}, {724, 805}, {962, 805}, {1200, 805}, {1438, 805}},
    {{10, 540}, {248, 540}, {486, 540}, {724, 540}, {962, 540}, {1200, 540}, {1438, 540}},
    {{10, 275}, {248, 275}, {486, 275}, {724, 275}, {962, 275}, {1200, 275}, {1438, 275}},
    {{10, 10}, {248, 10}, {486, 10}, {724, 10}, {962, 10}, {1200, 10}, {1438, 10}}};

void run(std::unordered_map<std::string, std::string> &buttonState, bool hasTriggers, Json::Value &config,
         int screenWidth, int screenHeight, SDL_Joystick *joystick) {
    int LEFT_JS_X_ID = config["left_joystick_x_id"].asInt();
    int LEFT_JS_Y_ID = config["left_joystick_y_id"].asInt();
    int RIGHT_JS_X_ID = config["right_joystick_x_id"].asInt();
    int RIGHT_JS_Y_ID = config["right_joystick_y_id"].asInt();
    int LEFT_TRIGGER_ID = config["left_trigger_id"].asInt();
    int RIGHT_TRIGGER_ID = config["right_trigger_id"].asInt();
    float LEFT_JS_DEAD_ZONE = config["left_joystick_dead_zone"].asFloat();
    float RIGHT_JS_DEAD_ZONE = config["right_joystick_dead_zone"].asFloat();
    float RIGHT_TRIGGER_DEAD_ZONE = config["right_trigger_dead_zone"].asFloat();
    float LEFT_TRIGGER_DEAD_ZONE = config["left_trigger_dead_zone"].asFloat();
    float RIGHT_JS_SENSITIVITY = config["right_joystick_sensitivity"].asFloat();
    std::unordered_map<int, std::string> buttonMapping;
    for (const auto &configKey : config["button_mapping"].getMemberNames()) {
        int key = config["button_mapping"][configKey].asInt() - 1;
        buttonMapping[key] = configKey;
    }
    bool running = config["run_automatically"].asBool();

    Functions functions;

    int center_x = screenWidth / 2;
    int center_y = screenHeight / 2;

    MouseMovementWithPadMode mode = BOARD;
    tft::State state = {2, 3};

    auto INPUT_TO_KEY_TAP = std::unordered_map<std::string, WORD>{{"B", 'E'}, {"X", 'F'}, {"Y", 'D'}, {"SELECT", 'W'}, {"R2", 'R'}, {"L2", 'Q'}};

    auto INPUT_TO_MOUSE_CLICK = std::unordered_map<std::string, int>{{"START", SDL_BUTTON_LEFT}};

    auto INPUT_TO_MOUSE_MOVE = std::unordered_map<std::string, std::pair<int, int> *>{
        {"LEFT", &state.mouse_target},
        {"RIGHT", &state.mouse_target},
        {"UP", &state.mouse_target},
        {"DOWN", &state.mouse_target}};

    auto INPUT_TO_MOUSE_BUTTON_HOLD_OR_RELEASE = std::unordered_map<std::string, int>{{"A", SDL_BUTTON_LEFT}};

    auto RELEASE_TO_MOUSE_MOVE = std::unordered_map<std::string, std::pair<int, int>>{
        {"R1", tft::BOARD_COORDINATES[state.boardRow][state.boardColumn]},
        {"L1", tft::BOARD_COORDINATES[state.boardRow][state.boardColumn]},
    };

    auto INPUT_TO_LOGIC_BEFORE = std::unordered_map<std::string, std::function<void()>>{
        {"START", [&]() { functions.moveMouse(center_x, center_y); }},
        {"LEFT", [&]() { updateAbstractState(mode, tft::PadDirections::LEFT, state); }},
        {"RIGHT", [&]() { updateAbstractState(mode, tft::PadDirections::RIGHT, state); }},
        {"UP", [&]() { updateAbstractState(mode, tft::PadDirections::UP, state); }},
        {"DOWN", [&]() { updateAbstractState(mode, tft::PadDirections::DOWN, state); }}};

    functions.setMaps(&buttonState, &INPUT_TO_KEY_TAP, nullptr, nullptr, &INPUT_TO_MOUSE_MOVE, &INPUT_TO_MOUSE_CLICK, &INPUT_TO_MOUSE_BUTTON_HOLD_OR_RELEASE, &RELEASE_TO_MOUSE_MOVE, &INPUT_TO_LOGIC_BEFORE, nullptr, nullptr, nullptr);

    try {
        float leftX, leftY, rightX, rightY;
        bool isLeftXActive, isLeftYActive, isRightXActive, isRightYActive, isLeftTriggerAxisActive,
            isRightTriggerAxisActive;

        auto lastUpdateTime = std::chrono::steady_clock::now();

        while (true) {
            auto loopStartTime = std::chrono::steady_clock::now();
            std::vector<SDL_Event> events;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                events.push_back(event);
            }
            if (!running) {
                for (const auto &event : events) {
                    if (event.type == SDL_JOYBUTTONDOWN) {
                        std::string button = buttonMapping[event.jbutton.button];
                        if (button == "ACTIVATE") {
                            running = true;
                            break;
                        }
                    }
                }
            } else {
                // state
                for (auto &pair : buttonState) {
                    std::string &state = pair.second;
                    if (state == "JUST_PRESSED") {
                        state = "PRESSED";
                    } else if (state == "JUST_RELEASED") {
                        state = "NOT_PRESSED";
                    }
                }

                leftX = SDL_JoystickGetAxis(joystick, LEFT_JS_X_ID) / 32768.0f;
                leftY = SDL_JoystickGetAxis(joystick, LEFT_JS_Y_ID) / 32768.0f;
                isLeftXActive = std::abs(leftX) > LEFT_JS_DEAD_ZONE;
                isLeftYActive = std::abs(leftY) > LEFT_JS_DEAD_ZONE;

                rightX = SDL_JoystickGetAxis(joystick, RIGHT_JS_X_ID) / 32768.0f;
                rightY = SDL_JoystickGetAxis(joystick, RIGHT_JS_Y_ID) / 32768.0f;
                isRightXActive = std::abs(rightX) > RIGHT_JS_DEAD_ZONE;
                isRightYActive = std::abs(rightY) > RIGHT_JS_DEAD_ZONE;

                if (hasTriggers) {
                    functions.handleState(&buttonState["R2"],
                                          (SDL_JoystickGetAxis(joystick, RIGHT_TRIGGER_ID) + 32768) / 65536.0f >
                                              RIGHT_TRIGGER_DEAD_ZONE);
                    functions.handleState(&buttonState["L2"],
                                          (SDL_JoystickGetAxis(joystick, LEFT_TRIGGER_ID) + 32768) / 65536.0f >
                                              LEFT_TRIGGER_DEAD_ZONE);
                }

                for (const auto &event : events) {
                    if (event.type == SDL_JOYBUTTONDOWN) {
                        std::string button = buttonMapping[event.jbutton.button];
                        functions.handleState(&buttonState[button], true);
                    } else if (event.type == SDL_JOYBUTTONUP) {
                        std::string button = buttonMapping[event.jbutton.button];
                        functions.handleState(&buttonState[button], false);
                    } else if (event.type == SDL_JOYHATMOTION) {
                        functions.handleState(&buttonState["LEFT"], event.jhat.value == SDL_HAT_LEFT);
                        functions.handleState(&buttonState["RIGHT"], event.jhat.value == SDL_HAT_RIGHT);
                        functions.handleState(&buttonState["DOWN"], event.jhat.value == SDL_HAT_DOWN);
                        functions.handleState(&buttonState["UP"], event.jhat.value == SDL_HAT_UP);
                    }
                }

                if (buttonState["ACTIVATE"] == "JUST_PRESSED") {
                    running = false;
                    continue;
                }

                // action
                for (const auto &[input, _] : INPUT_TO_KEY_TAP) {
                    functions.handleToKeyTapInput(input);
                }
                for (const auto &[input, _] : INPUT_TO_MOUSE_MOVE) {
                    functions.handleToMouseAbsoluteMoveInput(input);
                }
                for (const auto &[input, _] : RELEASE_TO_MOUSE_MOVE) {
                    functions.handleToMouseMoveRelease(input);
                }
                for (const auto &[input, _] : INPUT_TO_MOUSE_BUTTON_HOLD_OR_RELEASE) {
                    functions.handleToClickAndHoldOrReleaseInput(input);
                }
                for (const auto &[input, _] : INPUT_TO_MOUSE_CLICK) {
                    functions.handleToClickInput(input);
                }

                if (isRightXActive || isRightYActive) {
                    // ...
                }
                if (isLeftXActive || isLeftYActive) {
                    // ...
                }
            }

            std::this_thread::sleep_for(
                std::chrono::microseconds(std::max(10000 - std::chrono::duration_cast<std::chrono::microseconds>(
                                                               std::chrono::steady_clock::now() - loopStartTime)
                                                               .count(),
                                                   0LL)));
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void updateAbstractState(MouseMovementWithPadMode mode, tft::PadDirections direction, State &state) {
    switch (mode) {
    case BOARD:
        int row;
        int column;
        switch (direction) {
        case tft::PadDirections::UP:
            row = (state.boardRow + 1) % 5;
            column;
            if (row != 0) {
                column = state.boardColumn > 6 ? 6 : state.boardColumn;
            } else {
                column = state.boardColumn;
            }
            break;
        case tft::PadDirections::DOWN:
            row = (state.boardRow + 4) % 5;
            column;
            if (row != 0) {
                column = state.boardColumn > 6 ? 6 : state.boardColumn;
            } else {
                column = state.boardColumn;
            }
            break;
        case tft::PadDirections::LEFT:
            row = state.boardRow;
            if (row != 0) {
                column = (state.boardColumn + 6) % 7;
            } else {
                column = (state.boardColumn + 8) % 9;
            }
            break;
        case tft::PadDirections::RIGHT:
            row = state.boardRow;
            if (row != 0) {
                column = (state.boardColumn + 1) % 7;
            } else {
                column = (state.boardColumn + 1) % 9;
            }
            break;
        default:
            row = state.boardColumn;
            column = state.boardColumn;
        }
        state.boardRow = row;
        state.boardColumn = column;
        state.mouse_target = tft::BOARD_COORDINATES[state.boardRow][state.boardColumn];
        break;

    default:
        break;
    }
}
} // namespace tft
