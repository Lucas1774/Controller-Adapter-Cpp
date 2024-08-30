#include "funcs.h"
#include <chrono>
#include <functional>
#include <thread>
#include <windows.h>

void Functions::setMaps(std::unordered_map<int, int> *buttonState,
                        std::unordered_map<int, std::pair<int, int> *> *input_to_mouse_move,
                        std::unordered_map<int, std::pair<int, int> *> *release_to_mouse_move,
                        std::unordered_map<int, int> *input_to_mouse_click,
                        std::unordered_map<int, int> *release_to_mouse_click,
                        std::unordered_map<int, int> *input_to_button_toggle,
                        std::unordered_map<int, int> *release_to_button_toggle,
                        std::unordered_map<int, WORD> *input_to_key_tap,
                        std::unordered_map<int, WORD> *release_to_key_tap,
                        std::unordered_map<int, WORD> *input_to_key_hold,
                        std::unordered_map<int, std::function<void()>> *input_to_logic_before,
                        std::unordered_map<int, std::function<void()>> *input_to_logic_after,
                        std::unordered_map<int, std::function<void()>> *release_to_logic_before,
                        std::unordered_map<int, std::function<void()>> *release_to_logic_after) {
    this->buttonState = buttonState;
    this->input_to_mouse_move = input_to_mouse_move;
    this->release_to_mouse_move = release_to_mouse_move;
    this->input_to_mouse_click = input_to_mouse_click;
    this->release_to_mouse_click = release_to_mouse_click;
    this->input_to_button_toggle = input_to_button_toggle;
    this->release_to_button_toggle = release_to_button_toggle;
    this->input_to_key_tap = input_to_key_tap;
    this->release_to_key_tap = release_to_key_tap;
    this->input_to_key_hold = input_to_key_hold;
    this->input_to_logic_before = input_to_logic_before;
    this->input_to_logic_after = input_to_logic_after;
    this->release_to_logic_before = release_to_logic_before;
    this->release_to_logic_after = release_to_logic_after;
}

void Functions::sendInput(const int key, const DWORD flags) {
    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC);
    ip.ki.dwFlags = flags;
    SendInput(1, &ip, sizeof(INPUT));
}

void Functions::actionCallback(const int &input, const bool on_press, const bool before) {
    const auto *action_map = before
                                 ? (on_press ? this->input_to_logic_before : this->release_to_logic_before)
                                 : (on_press ? this->input_to_logic_after : this->release_to_logic_after);
    if (action_map != nullptr) {
        auto action = action_map->find(input);
        if (action != action_map->end()) {
            action->second();
        }
    }
}

void Functions::moveMouse(const int x, const int y) {
    SetCursorPos(x, y);
}

void Functions::moveMouseRelative(const int x, const int y) {
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x + x, p.y + y);
}

void Functions::click(const int button_to_click, const std::function<void()> callback) {
    INPUT ip[2] = {0};
    ip[0].type = INPUT_MOUSE;
    ip[0].mi.dwFlags = this->BUTTON_ID_TO_PRESS_EVENT[button_to_click];
    ip[1].type = INPUT_MOUSE;
    ip[1].mi.dwFlags = this->BUTOTN_ID_TO_RELEASE_EVENT[button_to_click];
    SendInput(2, ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::pressButton(const int button_to_press, const std::function<void()> callback) {
    INPUT ip = {0};
    ip.type = INPUT_MOUSE;
    ip.mi.dwFlags = BUTTON_ID_TO_PRESS_EVENT[button_to_press];
    SendInput(1, &ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::releaseButton(const int button_to_release, const std::function<void()> callback) {
    INPUT ip = {0};
    ip.type = INPUT_MOUSE;
    ip.mi.dwFlags = this->BUTOTN_ID_TO_RELEASE_EVENT[button_to_release];
    SendInput(1, &ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::pressThenRelease(const int key_to_tap, const std::function<void()> callback) {
    this->sendInput(key_to_tap, KEYEVENTF_SCANCODE);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    this->sendInput(key_to_tap, KEYEVENTF_KEYUP);
    if (callback) {
        callback();
    }
}

void Functions::handleToMouseAbsoluteMove(const int &input, const int eventType) {
    auto coordinates = PRESSED_STATES.find(eventType) != PRESSED_STATES.end() ? (*this->input_to_mouse_move)[input] : (*this->release_to_mouse_move)[input];
    if ((*this->buttonState)[input] == eventType) {
        bool on_press = PRESSED_STATES.find(eventType) != PRESSED_STATES.end();
        this->actionCallback(input, on_press, true);
        this->moveMouse(coordinates->first, coordinates->second);
        this->actionCallback(input, on_press, false);
    }
}

void Functions::handleToClick(const int &input, const int eventType) {
    int button = PRESSED_STATES.find(eventType) != PRESSED_STATES.end() ? (*this->input_to_mouse_click)[input] : (*this->release_to_mouse_click)[input];
    this->handleToClick(input, button, eventType);
}

void Functions::handleToClick(const int &input, const int button, const int eventType) {
    if ((*this->buttonState)[input] == eventType) {
        bool on_press = PRESSED_STATES.find(eventType) != PRESSED_STATES.end();
        this->actionCallback(input, on_press, true);
        std::thread([=] { this->click(button, [=] { this->actionCallback(input, on_press, false); }); })
            .detach();
    }
}

void Functions::handleToButtonToggle(const int &input, const int eventType) {
    int button = PRESSED_STATES.find(eventType) != PRESSED_STATES.end() ? (*this->input_to_button_toggle)[input] : (*this->release_to_button_toggle)[input];
    this->handleToButtonToggle(input, button, eventType);
}

void Functions::handleToButtonToggle(const int &input, const int button, const int eventType) {
    if ((*this->buttonState)[input] == eventType) {
        bool on_press = PRESSED_STATES.find(eventType) != PRESSED_STATES.end();
        this->actionCallback(input, on_press, true);
        if (!(GetAsyncKeyState(button) & 0x8000)) {
            std::thread([=] { this->pressButton(button, [=] { this->actionCallback(input, on_press, false); }); })
                .detach();
        } else {
            std::thread([=] { this->releaseButton(button, [=] { this->actionCallback(input, on_press, false); }); })
                .detach();
        }
    }
}

void Functions::handleToKeyTap(const int &input, const int eventType) {
    int key = PRESSED_STATES.find(eventType) != PRESSED_STATES.end() ? (*this->input_to_key_tap)[input] : (*this->release_to_key_tap)[input];
    this->handleToKeyTap(input, key, eventType);
}

void Functions::handleToKeyTap(const int &input, const int key, const int eventType) {
    if ((*this->buttonState)[input] == eventType) {
        bool on_press = PRESSED_STATES.find(eventType) != PRESSED_STATES.end();
        this->actionCallback(input, on_press, true);
        std::thread([=] { this->pressThenRelease(key, [=] { this->actionCallback(input, on_press, false); }); })
            .detach();
    }
}

void Functions::handleToKeyHold(const int &input) {
    int key = (*this->input_to_key_hold)[input]; // no on-release for this one
    this->handleToKeyHold(input, key);
}

void Functions::handleToKeyHold(const int &input, const int key) {
    int input_state = (*this->buttonState)[input];
    if (input_state == JUST_PRESSED) {
        this->actionCallback(input, true, true);
        this->sendInput(key, KEYEVENTF_SCANCODE);
        this->actionCallback(input, true, false);
    } else if (input_state == JUST_RELEASED) {
        this->actionCallback(input, false, true);
        this->sendInput(key, KEYEVENTF_KEYUP);
        this->actionCallback(input, false, false);
    }
}

void Functions::handleState(int *const state, const bool is_pressed) {
    if (is_pressed) {
        if (*state == RELEASED) {
            *state = JUST_PRESSED;
        }
    } else {
        if (*state == PRESSED) {
            *state = JUST_RELEASED;
        }
    }
}
