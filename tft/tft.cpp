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
    {{427, 756}, {544, 751}, {659, 757}, {776, 754}, {895, 756}, {1011, 754}, {1125, 754}, {1246, 752}, {1356, 752}},
    {{583, 632}, {699, 630}, {839, 634}, {960, 637}, {1096, 637}, {1214, 642}, {1340, 644}},
    {{535, 555}, {663, 559}, {783, 559}, {904, 565}, {1027, 564}, {1148, 561}, {1265, 565}},
    {{611, 482}, {727, 487}, {845, 489}, {961, 485}, {1081, 484}, {1189, 485}, {1314, 489}},
    {{567, 423}, {680, 426}, {794, 427}, {904, 427}, {1023, 429}, {1133, 422}, {1246, 420}}};

std::vector<std::pair<int, int>> SHOP_COORDINATES = {
    {300, 770}, {340, 735}, {320, 710}, {355, 675}, {410, 680}, {335, 645}, {390, 645}, {445, 645}, {350, 610}, {400, 610}};

const int BOARD_ADJACENCY_MATRIX[10][10] = {
    {NONE, UP, NONE, NONE, NONE, NONE, NONE, NONE, DOWN, NONE},  // 0
    {DOWN, NONE, UP, NONE, NONE, NONE, NONE, NONE, NONE, NONE},  // 1
    {NONE, DOWN, NONE, UP, NONE, NONE, NONE, NONE, NONE, NONE},  // 2
    {NONE, NONE, DOWN, NONE, RIGHT, UP, NONE, NONE, NONE, NONE}, // 3
    {NONE, NONE, DOWN, LEFT, NONE, NONE, UP, NONE, NONE, NONE},  // 4
    {NONE, NONE, NONE, DOWN, NONE, NONE, RIGHT, LEFT, UP, NONE}, // 5
    {NONE, NONE, NONE, NONE, DOWN, LEFT, NONE, RIGHT, NONE, UP}, // 6
    {NONE, NONE, NONE, NONE, DOWN, RIGHT, LEFT, NONE, NONE, UP}, // 7
    {UP, NONE, NONE, NONE, NONE, DOWN, NONE, NONE, NONE, RIGHT}, // 8
    {UP, NONE, NONE, NONE, NONE, NONE, DOWN, NONE, LEFT, NONE}}; // 9

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
    State state = {2, 3, 0, {}};

    auto INPUT_TO_KEY_TAP = std::unordered_map<std::string, WORD>{{"B", 'E'}, {"X", 'F'}, {"Y", 'D'}, {"SELECT", 'W'}, {"R2", 'R'}, {"L2", 'Q'}};

    auto INPUT_TO_MOUSE_CLICK = std::unordered_map<std::string, int>{{"START", SDL_BUTTON_LEFT}};

    auto INPUT_TO_MOUSE_MOVE = std::unordered_map<std::string, std::pair<int, int> *>{
        {"LEFT", &state.mouse_target},
        {"RIGHT", &state.mouse_target},
        {"UP", &state.mouse_target},
        {"DOWN", &state.mouse_target},
        {"R1", &SHOP_COORDINATES[0]},
        {"L1", &SHOP_COORDINATES[0]}};

    auto INPUT_TO_MOUSE_BUTTON_HOLD_OR_RELEASE = std::unordered_map<std::string, int>{{"A", SDL_BUTTON_LEFT}};

    auto RELEASE_TO_MOUSE_MOVE = std::unordered_map<std::string, std::pair<int, int>>{
        {"R1", BOARD_COORDINATES[state.boardRow][state.boardColumn]},
        {"L1", BOARD_COORDINATES[state.boardRow][state.boardColumn]},
    };

    auto INPUT_TO_LOGIC_BEFORE = std::unordered_map<std::string, std::function<void()>>{
        {"START", [&]() { functions.moveMouse(center_x, center_y); }},
        {"R1", [&]() { state.shopIndex = 0; }},
        {"L1", [&]() { state.shopIndex = 0; }},
        {"LEFT", [&]() { updateAbstractState(mode, Direction::LEFT, state); }},
        {"RIGHT", [&]() { updateAbstractState(mode, Direction::RIGHT, state); }},
        {"UP", [&]() { updateAbstractState(mode, Direction::UP, state); }},
        {"DOWN", [&]() { updateAbstractState(mode, Direction::DOWN, state); }}};

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
                if (buttonState["L1"] == "JUST_PRESSED") {
                    mode = ITEMS;
                } else if (buttonState["L1"] == "JUST_RELEASED") {
                    mode = BOARD;
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

void updateAbstractState(MouseMovementWithPadMode mode, Direction direction, State &state) {
    switch (mode) {
    case BOARD:
        int row;
        int column;
        switch (direction) {
        case Direction::UP:
            row = (state.boardRow + 1) % 5;
            column;
            if (row != 0) {
                column = state.boardColumn > 6 ? 6 : state.boardColumn;
            } else {
                column = state.boardColumn;
            }
            break;
        case Direction::DOWN:
            row = (state.boardRow + 4) % 5;
            column;
            if (row != 0) {
                column = state.boardColumn > 6 ? 6 : state.boardColumn;
            } else {
                column = state.boardColumn;
            }
            break;
        case Direction::LEFT:
            row = state.boardRow;
            if (row != 0) {
                column = (state.boardColumn + 6) % 7;
            } else {
                column = (state.boardColumn + 8) % 9;
            }
            break;
        case Direction::RIGHT:
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
        state.mouse_target = BOARD_COORDINATES[state.boardRow][state.boardColumn];
        break;

    case ITEMS:
        for (int i = 0; i < 10; i++) {
            if (BOARD_ADJACENCY_MATRIX[state.shopIndex][i] == direction) {
                state.shopIndex = i;
                state.mouse_target = SHOP_COORDINATES[state.shopIndex];
                break;
            }
        }
        break;

    default:
        break;
    }
}
} // namespace tft
