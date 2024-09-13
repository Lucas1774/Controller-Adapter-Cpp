#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

enum Buttons {
    A,
    B,
    X,
    Y,
    L1,
    R1,
    SELECT,
    START,
    L3,
    R3,
    XBOX,
    PAD_UP,
    PAD_LEFT,
    PAD_RIGHT,
    PAD_DOWN,
    L2,
    R2,
    ACTIVATE,
    LEFT_JS_LEFT,
    LEFT_JS_RIGHT,
    LEFT_JS_UP,
    LEFT_JS_DOWN,
    RIGHT_JS_LEFT,
    RIGHT_JS_RIGHT,
    RIGHT_JS_UP,
    RIGHT_JS_DOWN
};

const std::unordered_map<std::string, int> BUTTON_NAME_TO_BUTTON_ID = {
    {"A", A},
    {"B", B},
    {"X", X},
    {"Y", Y},
    {"L1", L1},
    {"R1", R1},
    {"SELECT", SELECT},
    {"START", START},
    {"L3", L3},
    {"R3", R3},
    {"XBOX", XBOX},
    {"UP", PAD_UP},
    {"LEFT", PAD_LEFT},
    {"RIGHT", PAD_RIGHT},
    {"DOWN", PAD_DOWN},
    {"L2", L2},
    {"R2", R2},
    {"ACTIVATE", ACTIVATE},
    {"LEFT_JS_LEFT", LEFT_JS_LEFT},
    {"LEFT_JS_RIGHT", LEFT_JS_RIGHT},
    {"LEFT_JS_UP", LEFT_JS_UP},
    {"LEFT_JS_DOWN", LEFT_JS_DOWN},
    {"RIGHT_JS_LEFT", RIGHT_JS_LEFT},
    {"RIGHT_JS_RIGHT", RIGHT_JS_RIGHT},
    {"RIGHT_JS_UP", RIGHT_JS_UP},
    {"RIGHT_JS_DOWN", RIGHT_JS_DOWN}};

enum ButtonState {
    PRESSED,
    JUST_PRESSED,
    JUST_RELEASED,
    RELEASED,
};

const std::unordered_set<int> PRESSED_STATES = {PRESSED, JUST_PRESSED};
const std::unordered_set<int> RELEASED_STATES = {RELEASED, JUST_RELEASED};