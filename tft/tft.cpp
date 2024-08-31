#include "tft.h"
#include "constants.h"
#include "funcs.h"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
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

const std::vector<std::pair<int, int>> ITEM_COORDINATES = {
    {283, 698}, {326, 663}, {280, 626}, {325, 615}, {379, 615}, {311, 580}, {351, 580}, {406, 579}, {310, 547}, {365, 543}};

const std::vector<std::pair<int, int>> SHOP_COORDINATES = {
    {574, 991}, {778, 989}, {980, 986}, {1182, 982}, {1370, 984}};

const std::vector<std::vector<std::pair<int, int>>> CARD_COORDINATES = {
    {{553, 580}, {963, 580}, {1380, 583}},
    {{552, 865}, {959, 866}, {1365, 865}}};

const std::vector<std::pair<int, int>> LOCK_COORDINATES = {{1450, 905}, {28, 350}, {33, 436}, {34, 528}, {356, 473}, {344, 538}, {349, 636}};

constexpr float MAX_RADIUS = 0.45;
constexpr float HORIZONTAL_RADIUS_OFFSET = 0.9; // the board is not proportional to the screen it sits on, this will help adapt to that

constexpr int BOARD_ADJACENCY_MATRIX[10][10] = {
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

constexpr int LOCK_ADJACENCY_MATRIX[7][7] = {
    {NONE, UP, RIGHT, LEFT, NONE, NONE, NONE}, // 0
    {LEFT, NONE, DOWN, UP, RIGHT, NONE, NONE}, // 1
    {LEFT, UP, NONE, DOWN, NONE, RIGHT, NONE}, // 2
    {LEFT, DOWN, UP, NONE, NONE, NONE, RIGHT}, // 3
    {RIGHT, LEFT, NONE, NONE, NONE, DOWN, UP}, // 4
    {RIGHT, NONE, LEFT, NONE, UP, NONE, DOWN}, // 5
    {RIGHT, NONE, NONE, LEFT, DOWN, UP, NONE}, // 6
};

void run(std::unordered_map<int, int> &buttonState,
         const bool &hasTriggers,
         const Json::Value &config,
         const int screenWidth,
         const int screenHeight,
         SDL_Joystick *joystick) {
    const int LEFT_JS_X_ID = config["left_joystick_x_id"].asInt();
    const int LEFT_JS_Y_ID = config["left_joystick_y_id"].asInt();
    const int RIGHT_JS_X_ID = config["right_joystick_x_id"].asInt();
    const int RIGHT_JS_Y_ID = config["right_joystick_y_id"].asInt();
    const int LEFT_TRIGGER_ID = config["left_trigger_id"].asInt();
    const int RIGHT_TRIGGER_ID = config["right_trigger_id"].asInt();
    const float LEFT_JS_DEAD_ZONE = config["left_joystick_dead_zone"].asFloat();
    const float RIGHT_JS_DEAD_ZONE = config["right_joystick_dead_zone"].asFloat();
    const float RIGHT_TRIGGER_DEAD_ZONE = config["right_trigger_dead_zone"].asFloat();
    const float LEFT_TRIGGER_DEAD_ZONE = config["left_trigger_dead_zone"].asFloat();
    const float RIGHT_JS_SENSITIVITY = config["right_joystick_sensitivity"].asFloat();
    std::unordered_map<int, int> buttonMapping;
    for (const std::string &configKey : config["button_mapping"].getMemberNames()) {
        int key = config["button_mapping"][configKey].asInt() - 1;
        buttonMapping[key] = BUTTON_NAME_TO_BUTTON_ID.at(configKey);
    }
    bool running = config["run_automatically"].asBool();

    Functions functions;

    const int center_x = screenWidth / 2;
    const int center_y = screenHeight / 2;
    const int vertical_adjustment = screenHeight / 10;
    const float res_scaling_x = static_cast<float>(screenWidth) / 1920;
    const float res_scaling_y = static_cast<float>(screenHeight) / 1080;
    const auto now = std::chrono::steady_clock::now();

    State state = {
        .boardRow = 2,
        .boardColumn = 3,
        .itemIndex = 0,
        .shopIndex = 2,
        .cardRow = 0,
        .cardColumn = 1,
        .lockIndex = 0,
        .mode = BOARD,
        .mouse_target = {},
        .pad_to_last_pressed = {{LEFT, now}, {RIGHT, now}, {UP, now}, {DOWN, now}},
        .pad_to_last_executed = {{LEFT, now}, {RIGHT, now}, {UP, now}, {DOWN, now}},
        .pad_to_is_unleashed = {{LEFT, false}, {RIGHT, false}, {UP, false}, {DOWN, false}}};

    const auto TURBO_INPUTS = std::unordered_set<int>{PAD_LEFT, PAD_RIGHT, PAD_UP, PAD_DOWN};

    const auto INPUT_TO_KEY_TAP = std::unordered_map<int, WORD>{{B, 'E'}, {X, 'F'}, {Y, 'D'}, {R2, 'R'}, {L2, 'Q'}};

    const auto INPUT_TO_MOUSE_CLICK = std::unordered_map<int, int>{{START, SDL_BUTTON_LEFT}, {A, SDL_BUTTON_LEFT}, {SELECT, SDL_BUTTON_RIGHT}};

    const auto INPUT_TO_MOUSE_MOVE = std::unordered_map<int, std::pair<int, int> *>{
        {PAD_LEFT, &state.mouse_target},
        {PAD_RIGHT, &state.mouse_target},
        {PAD_UP, &state.mouse_target},
        {PAD_DOWN, &state.mouse_target},
        {R1, &state.mouse_target},
        {L1, &state.mouse_target},
        {R3, &state.mouse_target},
        {L3, &state.mouse_target}};

    const auto RELEASE_TO_MOUSE_MOVE = std::unordered_map<int, std::pair<int, int> *>{
        {R1, &state.mouse_target},
        {L1, &state.mouse_target},
    };

    const auto INPUT_TO_LOGIC_BEFORE = std::unordered_map<int, std::function<void()>>{
        {START, [&]() { functions.moveMouse(956 * res_scaling_x, 993 * res_scaling_y); }},
        {PAD_LEFT, [&]() { updateAbstractState(LEFT, buttonState.at(PAD_LEFT), state, res_scaling_x, res_scaling_y); }},
        {PAD_RIGHT, [&]() { updateAbstractState(RIGHT, buttonState[PAD_RIGHT], state, res_scaling_x, res_scaling_y); }},
        {PAD_UP, [&]() { updateAbstractState(UP, buttonState[PAD_UP], state, res_scaling_x, res_scaling_y); }},
        {PAD_DOWN, [&]() { updateAbstractState(DOWN, buttonState[PAD_DOWN], state, res_scaling_x, res_scaling_y); }}};

    functions.setMaps(&buttonState, &INPUT_TO_MOUSE_MOVE, &RELEASE_TO_MOUSE_MOVE, &INPUT_TO_MOUSE_CLICK, nullptr, nullptr, nullptr, &INPUT_TO_KEY_TAP, nullptr, nullptr, &INPUT_TO_LOGIC_BEFORE, nullptr, nullptr, nullptr);

    try {
        float leftX, leftY, rightX, rightY;
        bool isLeftXActive, isLeftYActive, isRightXActive, isRightYActive, isLeftTriggerAxisActive, isRightTriggerAxisActive;

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
                        int button = buttonMapping[event.jbutton.button];
                        if (button == ACTIVATE) {
                            running = true;
                            break;
                        }
                    }
                }
            } else {
                // state
                for (auto &pair : buttonState) {
                    int &state = pair.second;
                    if (state == JUST_PRESSED) {
                        state = PRESSED;
                    } else if (state == JUST_RELEASED) {
                        state = RELEASED;
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
                    functions.handleState(buttonState[R2],
                                          (SDL_JoystickGetAxis(joystick, RIGHT_TRIGGER_ID) + 32768) / 65536.0f >
                                              RIGHT_TRIGGER_DEAD_ZONE);
                    functions.handleState(buttonState[L2],
                                          (SDL_JoystickGetAxis(joystick, LEFT_TRIGGER_ID) + 32768) / 65536.0f >
                                              LEFT_TRIGGER_DEAD_ZONE);
                }

                for (const auto &event : events) {
                    if (event.type == SDL_JOYBUTTONDOWN) {
                        int button = buttonMapping[event.jbutton.button];
                        functions.handleState(buttonState[button], true);
                    } else if (event.type == SDL_JOYBUTTONUP) {
                        int button = buttonMapping[event.jbutton.button];
                        functions.handleState(buttonState[button], false);
                    } else if (event.type == SDL_JOYHATMOTION) {
                        functions.handleState(buttonState[PAD_LEFT], event.jhat.value == SDL_HAT_LEFT);
                        functions.handleState(buttonState[PAD_RIGHT], event.jhat.value == SDL_HAT_RIGHT);
                        functions.handleState(buttonState[PAD_DOWN], event.jhat.value == SDL_HAT_DOWN);
                        functions.handleState(buttonState[PAD_UP], event.jhat.value == SDL_HAT_UP);
                    }
                }

                if (buttonState[ACTIVATE] == JUST_PRESSED) {
                    running = false;
                    continue;
                }
                if (buttonState[L1] == JUST_PRESSED) {
                    state.mode = ITEMS;
                    auto coordinates = ITEM_COORDINATES[state.itemIndex];
                    state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
                } else if (buttonState[L1] == JUST_RELEASED) {
                    state.mode = BOARD;
                    auto coordinates = BOARD_COORDINATES[state.boardRow][state.boardColumn];
                    state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
                } else if (buttonState[R1] == JUST_PRESSED) {
                    state.mode = SHOP;
                    auto coordinates = SHOP_COORDINATES[state.shopIndex];
                    state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
                } else if (buttonState[R1] == JUST_RELEASED) {
                    state.mode = BOARD;
                    auto coordinates = BOARD_COORDINATES[state.boardRow][state.boardColumn];
                    state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
                } else if (buttonState[R3] == JUST_PRESSED) {
                    if (state.mode == CARDS) {
                        state.mode = BOARD;
                        auto coordinates = BOARD_COORDINATES[state.boardRow][state.boardColumn];
                        state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
                    } else {
                        state.mode = CARDS;
                        state.cardRow = 0; // do not memorize previous location for cards (risky business)
                        state.cardColumn = 1;
                        auto coordinates = CARD_COORDINATES[state.cardRow][state.cardColumn];
                        state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
                    }
                } else if (buttonState[L3] == JUST_PRESSED) {
                    if (state.mode == LOCK) {
                        state.mode = BOARD;
                        auto coordinates = BOARD_COORDINATES[state.boardRow][state.boardColumn];
                        state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
                    } else {
                        state.mode = LOCK;
                        state.lockIndex = 0; // do not memorize here either because why would you
                        auto coordinates = LOCK_COORDINATES[state.lockIndex];
                        state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
                    }
                }

                // action
                else if (buttonState[START] == JUST_PRESSED) {
                    functions.moveMouse(956 * res_scaling_x, 993 * res_scaling_y);
                }
                for (const auto &[input, _] : INPUT_TO_KEY_TAP) {
                    functions.handleToKeyTap(input, JUST_PRESSED);
                }
                for (const auto &[input, _] : INPUT_TO_MOUSE_MOVE) {
                    if (TURBO_INPUTS.find(input) != TURBO_INPUTS.end()) {
                        functions.handleToMouseAbsoluteMove(input, PRESSED);
                    }
                    functions.handleToMouseAbsoluteMove(input, JUST_PRESSED);
                }
                for (const auto &[input, _] : RELEASE_TO_MOUSE_MOVE) {
                    functions.handleToMouseAbsoluteMove(input, JUST_RELEASED);
                }
                for (const auto &[input, _] : INPUT_TO_MOUSE_CLICK) {
                    functions.handleToClick(input, JUST_PRESSED);
                }

                if (isRightXActive || isRightYActive) {
                    if (std::chrono::steady_clock::now() - lastUpdateTime > std::chrono::milliseconds(100)) {
                        state.mode = FREE;
                        functions.moveMouseRelative(
                            round(rightX * RIGHT_JS_SENSITIVITY * 500 * res_scaling_x),
                            round(rightY * RIGHT_JS_SENSITIVITY * 500 * res_scaling_y));
                        lastUpdateTime = std::chrono::steady_clock::now();
                    }
                } else if (isLeftXActive || isLeftYActive) {
                    functions.moveMouse(
                        round(leftX * center_x * MAX_RADIUS * HORIZONTAL_RADIUS_OFFSET * res_scaling_x + center_x),
                        round((leftY * center_y * MAX_RADIUS * res_scaling_y + center_y) - vertical_adjustment));
                    if (std::chrono::steady_clock::now() - lastUpdateTime > std::chrono::milliseconds(100)) {
                        functions.click(SDL_BUTTON_RIGHT);
                        lastUpdateTime = std::chrono::steady_clock::now();
                    }
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

void updateAbstractState(const int direction, const int &buttonState, State &state, const float res_scaling_x, const float res_scaling_y) {
    auto now = std::chrono::steady_clock::now();
    auto last_executed = state.pad_to_last_executed[direction];
    if (now - last_executed > std::chrono::milliseconds(50)) {
        auto is_unleashed = state.pad_to_is_unleashed[direction];
        auto last_pressed = state.pad_to_last_pressed[direction];
        if (is_unleashed || now - last_pressed > std::chrono::milliseconds(200)) {
            state.pad_to_last_executed[direction] = now;
            state.pad_to_is_unleashed[direction] = buttonState == PRESSED;
            if (buttonState == JUST_PRESSED) {
                state.pad_to_last_pressed[direction] = now;
            }
            if (state.mode == FREE) {
                state.mode = BOARD;
                return;
            }
            std::pair<int, int> coordinates;
            if (state.mode == BOARD) {
                if (direction == UP) {
                    state.boardRow = (state.boardRow + 1) % 5;
                    if (state.boardRow != 0) {
                        state.boardColumn = state.boardColumn > 6 ? 6 : state.boardColumn;
                    }
                } else if (direction == DOWN) {
                    state.boardRow = (state.boardRow + 4) % 5;
                    if (state.boardRow != 0) {
                        state.boardColumn = state.boardColumn > 6 ? 6 : state.boardColumn;
                    }
                } else if (direction == LEFT) {
                    if (state.boardRow != 0) {
                        state.boardColumn = (state.boardColumn + 6) % 7;
                    } else {
                        state.boardColumn = (state.boardColumn + 8) % 9;
                    }
                } else if (direction == RIGHT) {
                    if (state.boardRow != 0) {
                        state.boardColumn = (state.boardColumn + 1) % 7;
                    } else {
                        state.boardColumn = (state.boardColumn + 1) % 9;
                    }
                }
                coordinates = BOARD_COORDINATES[state.boardRow][state.boardColumn];
            } else if (state.mode == ITEMS) {
                for (int i = 0; i < 10; i++) {
                    if (BOARD_ADJACENCY_MATRIX[state.itemIndex][i] == direction) {
                        state.itemIndex = i;
                        break;
                    }
                }
                coordinates = ITEM_COORDINATES[state.itemIndex];
            } else if (state.mode == SHOP) {
                if (direction == LEFT) {
                    state.shopIndex = (state.shopIndex + 4) % 5;
                } else if (direction == RIGHT) {
                    state.shopIndex = (state.shopIndex + 1) % 5;
                }
                coordinates = SHOP_COORDINATES[state.shopIndex];
            } else if (state.mode == CARDS) {
                if (direction == UP || direction == DOWN) {
                    state.cardRow = (state.cardRow + 1) % 2;
                } else if (direction == RIGHT) {
                    state.cardColumn = (state.cardColumn + 1) % 3;
                } else if (direction == LEFT) {
                    state.cardColumn = (state.cardColumn + 2) % 3;
                }
                coordinates = CARD_COORDINATES[state.cardRow][state.cardColumn];
            } else if (state.mode == LOCK) {
                for (int i = 0; i < 7; i++) {
                    if (LOCK_ADJACENCY_MATRIX[state.lockIndex][i] == direction) {
                        state.lockIndex = i;
                        break;
                    }
                }
                coordinates = LOCK_COORDINATES[state.lockIndex];
            }
            state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
        }
    }
}
} // namespace tft
