#include "chess.h"
#include "constants.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>
#include <windows.h>

namespace chess {
const std::vector<std::vector<std::pair<int, int>>> BOARD_COORDINATES = {
    {{304, 929}, {413, 928}, {511, 928}, {624, 925}, {725, 924}, {829, 925}, {937, 926}, {1039, 928}},
    {{308, 819}, {412, 822}, {518, 820}, {623, 822}, {724, 824}, {830, 822}, {934, 823}, {1042, 823}},
    {{295, 712}, {411, 713}, {510, 709}, {620, 712}, {724, 703}, {830, 712}, {930, 709}, {1035, 709}},
    {{295, 607}, {409, 606}, {512, 610}, {619, 605}, {718, 607}, {829, 606}, {931, 603}, {1034, 605}},
    {{294, 499}, {405, 501}, {506, 499}, {618, 496}, {719, 498}, {828, 497}, {936, 497}, {1035, 497}},
    {{299, 391}, {409, 393}, {513, 389}, {615, 390}, {719, 391}, {829, 391}, {935, 399}, {1034, 394}},
    {{306, 295}, {409, 295}, {511, 295}, {616, 295}, {725, 295}, {826, 299}, {933, 296}, {1037, 296}},
    {{304, 190}, {412, 190}, {517, 189}, {615, 189}, {722, 187}, {830, 190}, {935, 193}, {1044, 192}}};

const std::vector<std::pair<int, int>> RESIGN_YES_NO = {
    {1176, 510}, {1120, 505}};

const std::vector<std::pair<int, int>> DRAW_YES_NO = {
    {1182, 450}, {1118, 448}};

void run(std::unordered_map<int, int> &buttonState,
         const bool &hasTriggers,
         const Json::Value &config,
         const int screenWidth,
         const int screenHeight,
         SDL_Joystick *joystick) {
    std::unordered_map<int, int> buttonMapping;
    for (const auto &configKey : config["button_mapping"].getMemberNames()) {
        int key = config["button_mapping"][configKey].asInt() - 1;
        buttonMapping[key] = BUTTON_NAME_TO_BUTTON_ID.at(configKey);
    }
    bool running = config["run_automatically"].asBool();

    Functions functions;
    const auto PLAY_AGAIN_POS = std::make_pair(1255, 539);
    const auto REMATCH_POS = std::make_pair(1431, 536);
    const auto RESIGN_POS = std::make_pair(1177, 570);
    const auto DRAW_POS = std::make_pair(1163, 511);
    const auto TURBO_INPUTS = std::unordered_set<int>{PAD_LEFT, PAD_RIGHT, PAD_UP, PAD_DOWN};

    const float res_scaling_x = static_cast<float>(screenWidth) / 1920;
    const float res_scaling_y = static_cast<float>(screenHeight) / 1080;
    const auto now = std::chrono::steady_clock::now();

    State state = {
        .boardRow = 0,
        .boardColumn = 0,
        .resignIndex = 0,
        .drawIndex = 0,
        .mode = BOARD,
        .mouse_target = {},
    };
    BufferState bufferState = {
        .last_pressed = now,
        .last_executed = now,
        .is_unleashed = false,
    };

    const auto INPUT_TO_MOUSE_MOVE = std::unordered_map<int, std::pair<int, int> *>{
        {PAD_LEFT, &state.mouse_target},
        {PAD_RIGHT, &state.mouse_target},
        {PAD_UP, &state.mouse_target},
        {PAD_DOWN, &state.mouse_target}};
    const auto INPUT_TO_BUTTON_CLICK = std::unordered_map<int, int>{{B, SDL_BUTTON_RIGHT}, {R1, SDL_BUTTON_LEFT}, {L1, SDL_BUTTON_LEFT}, {Y, SDL_BUTTON_LEFT}, {X, SDL_BUTTON_LEFT}};
    const auto INPUT_TO_BUTTON_TOGGLE = std::unordered_map<int, int>{{A, SDL_BUTTON_LEFT}};
    const auto RELEASE_TO_BUTTON_TOGGLE = std::unordered_map<int, int>{{A, SDL_BUTTON_LEFT}};
    const auto INPUT_TO_LOGIC_BEFORE = std::unordered_map<int, std::function<bool()>>{
        {L1, [&]() { functions.moveMouse(REMATCH_POS.first * res_scaling_x, REMATCH_POS.second * res_scaling_y); return true; }},
        {R1, [&]() { functions.moveMouse(PLAY_AGAIN_POS.first * res_scaling_x, PLAY_AGAIN_POS.second * res_scaling_y); return true; }},
        {X, [&]() { functions.moveMouse(DRAW_POS.first * res_scaling_x, DRAW_POS.second * res_scaling_y); return true; }},
        {Y, [&]() { functions.moveMouse(RESIGN_POS.first * res_scaling_x, RESIGN_POS.second * res_scaling_y); return true; }},
        {PAD_LEFT, [&]() { return updateAbstractState(PAD_LEFT, buttonState[PAD_LEFT], state, bufferState, res_scaling_x, res_scaling_y, functions); }},
        {PAD_RIGHT, [&]() { return updateAbstractState(PAD_RIGHT, buttonState[PAD_RIGHT], state, bufferState, res_scaling_x, res_scaling_y, functions); }},
        {PAD_UP, [&]() { return updateAbstractState(PAD_UP, buttonState[PAD_UP], state, bufferState, res_scaling_x, res_scaling_y, functions); }},
        {PAD_DOWN, [&]() { return updateAbstractState(PAD_DOWN, buttonState[PAD_DOWN], state, bufferState, res_scaling_x, res_scaling_y, functions); }},
    };
    const auto INPUT_TO_LOGIC_AFTER = std::unordered_map<int, std::function<bool()>>{
        {L1, [&]() { auto c = BOARD_COORDINATES[state.boardRow][state.boardColumn]; functions.moveMouse(c.first * res_scaling_x, c.second * res_scaling_y); return true; }},
        {R1, [&]() { auto c = BOARD_COORDINATES[state.boardRow][state.boardColumn]; functions.moveMouse(c.first * res_scaling_x, c.second * res_scaling_y); return true; }},
        {X, [&]() { state.mode = DRAW; auto c = DRAW_YES_NO[state.drawIndex]; functions.moveMouse(c.first * res_scaling_x, c.second * res_scaling_y); return true; }},
        {Y, [&]() { state.mode = RESIGN; auto c = RESIGN_YES_NO[state.resignIndex]; functions.moveMouse(c.first * res_scaling_x, c.second * res_scaling_y); return true; }},
        {A, [&]() { if (state.mode != BOARD) {state.mode = BOARD; state.drawIndex = 0; state.resignIndex = 0;} return true; }},
    };

    functions.setMaps(&buttonState, &INPUT_TO_MOUSE_MOVE, nullptr, &INPUT_TO_BUTTON_CLICK, nullptr, &INPUT_TO_BUTTON_TOGGLE, &RELEASE_TO_BUTTON_TOGGLE, nullptr, nullptr, nullptr, &INPUT_TO_LOGIC_BEFORE, &INPUT_TO_LOGIC_AFTER, nullptr, nullptr);

    try {
        while (true) {
            auto loop_start_time = std::chrono::steady_clock::now();
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

                // action
                for (const auto &[input, _] : INPUT_TO_MOUSE_MOVE) {
                    if (TURBO_INPUTS.find(input) != TURBO_INPUTS.end()) {
                        functions.handleToMouseAbsoluteMove(input, PRESSED);
                    }
                    functions.handleToMouseAbsoluteMove(input, JUST_PRESSED);
                }
                for (const auto &[input, _] : INPUT_TO_BUTTON_CLICK) {
                    functions.handleToClick(input, JUST_PRESSED);
                }
                for (const auto &[input, _] : INPUT_TO_BUTTON_TOGGLE) {
                    functions.handleToButtonToggle(input, JUST_PRESSED);
                }
                for (const auto &[input, _] : RELEASE_TO_BUTTON_TOGGLE) {
                    functions.handleToButtonToggle(input, JUST_RELEASED);
                }
            }

            std::this_thread::sleep_for(std::chrono::microseconds(std::max(
                10000 - std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - loop_start_time).count(), 0LL)));
        }
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

bool updateAbstractState(const int button, const int &buttonState, State &state, BufferState &bufferState, const float res_scaling_x, const float res_scaling_y, const Functions &functions) {
    if (!functions.isBufferFree(200, 50, buttonState, bufferState)) {
        return false;
    }

    std::pair<int, int> coordinates;
    if (state.mode == BOARD) {
        if (button == PAD_UP) {
            state.boardRow = (state.boardRow + 1) % 8;
        } else if (button == PAD_DOWN) {
            state.boardRow = (state.boardRow + 7) % 8;
        } else if (button == PAD_LEFT) {
            state.boardColumn = (state.boardColumn + 7) % 8;
        } else if (button == PAD_RIGHT) {
            state.boardColumn = (state.boardColumn + 1) % 8;
        }
        coordinates = BOARD_COORDINATES[state.boardRow][state.boardColumn];
    } else if (state.mode == RESIGN) {
        if (button == PAD_LEFT || button == PAD_RIGHT) {
            state.resignIndex = 1 - state.resignIndex;
            coordinates = RESIGN_YES_NO[state.resignIndex];
        } else {
            return false;
        }
    } else if (state.mode == DRAW) {
        if (button == PAD_LEFT || button == PAD_RIGHT) {
            state.drawIndex = 1 - state.drawIndex;
            coordinates = DRAW_YES_NO[state.drawIndex];
        } else {
            return false;
        }
    }
    state.mouse_target = {coordinates.first * res_scaling_x, coordinates.second * res_scaling_y};
    return true;
}
} // namespace chess
