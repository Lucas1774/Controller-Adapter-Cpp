#include "funcs.h"
#include <functional>
#include <thread>
#include <windows.h>

void Functions::setMaps(const std::unordered_map<int, int> *buttonState,
                        const std::unordered_map<int, std::pair<int, int> *> *input_to_mouse_move,
                        const std::unordered_map<int, std::pair<int, int> *> *release_to_mouse_move,
                        const std::unordered_map<int, int> *input_to_mouse_click,
                        const std::unordered_map<int, int> *release_to_mouse_click,
                        const std::unordered_map<int, int> *input_to_button_toggle,
                        const std::unordered_map<int, int> *release_to_button_toggle,
                        const std::unordered_map<int, WORD> *input_to_key_tap,
                        const std::unordered_map<int, WORD> *release_to_key_tap,
                        const std::unordered_map<int, WORD> *input_to_key_hold,
                        const std::unordered_map<int, std::function<bool()>> *input_to_logic_before,
                        const std::unordered_map<int, std::function<bool()>> *input_to_logic_after,
                        const std::unordered_map<int, std::function<bool()>> *release_to_logic_before,
                        const std::unordered_map<int, std::function<bool()>> *release_to_logic_after) {
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

void Functions::sendInput(const int key, const DWORD flags) const {
    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC);
    ip.ki.dwFlags = flags;
    SendInput(1, &ip, sizeof(INPUT));
}

bool Functions::actionCallback(const int &input, const bool on_press, const bool before) const {
    const auto *action_map = before
                                 ? (on_press ? this->input_to_logic_before : this->release_to_logic_before)
                                 : (on_press ? this->input_to_logic_after : this->release_to_logic_after);
    bool result = true;
    if (action_map != nullptr) {
        auto action = action_map->find(input);
        if (action != action_map->end()) {
            result = action->second();
        }
    }
    return result;
}

void Functions::moveMouse(const int x, const int y) const {
    SetCursorPos(x, y);
}

void Functions::moveMouseRelative(const int x, const int y) const {
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x + x, p.y + y);
}

void Functions::click(const int button_to_click, const std::function<void()> callback) const {
    INPUT ip[2] = {0};
    ip[0].type = INPUT_MOUSE;
    ip[0].mi.dwFlags = this->BUTTON_ID_TO_PRESS_EVENT.at(button_to_click);
    ip[1].type = INPUT_MOUSE;
    ip[1].mi.dwFlags = this->BUTOTN_ID_TO_RELEASE_EVENT.at(button_to_click);
    SendInput(2, ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::pressButton(const int button_to_press, const std::function<void()> callback) const {
    INPUT ip = {0};
    ip.type = INPUT_MOUSE;
    ip.mi.dwFlags = BUTTON_ID_TO_PRESS_EVENT.at(button_to_press);
    SendInput(1, &ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::releaseButton(const int button_to_release, const std::function<void()> callback) const {
    INPUT ip = {0};
    ip.type = INPUT_MOUSE;
    ip.mi.dwFlags = this->BUTOTN_ID_TO_RELEASE_EVENT.at(button_to_release);
    SendInput(1, &ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::pressThenRelease(const int key_to_tap, const std::function<void()> callback) const {
    this->sendInput(key_to_tap, KEYEVENTF_SCANCODE);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    this->sendInput(key_to_tap, KEYEVENTF_KEYUP);
    if (callback) {
        callback();
    }
}

void Functions::handleToMouseAbsoluteMove(const int &input, const int eventType) const {
    auto coordinates = PRESSED_STATES.find(eventType) != PRESSED_STATES.end() ? (*this->input_to_mouse_move).at(input) : (*this->release_to_mouse_move).at(input);
    if ((*this->buttonState).at(input) == eventType) {
        bool on_press = PRESSED_STATES.find(eventType) != PRESSED_STATES.end();
        if (this->actionCallback(input, on_press, true)) {
            this->moveMouse(coordinates->first, coordinates->second);
            this->actionCallback(input, on_press, false);
        }
    }
}

void Functions::handleToClick(const int &input, const int eventType) const {
    int button = PRESSED_STATES.find(eventType) != PRESSED_STATES.end() ? (*this->input_to_mouse_click).at(input) : (*this->release_to_mouse_click).at(input);
    this->handleToClick(input, button, eventType);
}

void Functions::handleToClick(const int &input, const int button, const int eventType) const {
    if ((*this->buttonState).at(input) == eventType) {
        bool on_press = PRESSED_STATES.find(eventType) != PRESSED_STATES.end();
        if (this->actionCallback(input, on_press, true)) {
            std::thread([=] { this->click(button, [=] { this->actionCallback(input, on_press, false); }); })
                .detach();
        }
    }
}

void Functions::handleToButtonToggle(const int &input, const int eventType) const {
    int button = PRESSED_STATES.find(eventType) != PRESSED_STATES.end() ? (*this->input_to_button_toggle).at(input) : (*this->release_to_button_toggle).at(input);
    this->handleToButtonToggle(input, button, eventType);
}

void Functions::handleToButtonToggle(const int &input, const int button, const int eventType) const {
    if ((*this->buttonState).at(input) == eventType) {
        bool on_press = PRESSED_STATES.find(eventType) != PRESSED_STATES.end();
        if (this->actionCallback(input, on_press, true)) {
            if (!(GetAsyncKeyState(button) & 0x8000)) {
                std::thread([=] { this->pressButton(button, [=] { this->actionCallback(input, on_press, false); }); })
                    .detach();
            } else {
                std::thread([=] { this->releaseButton(button, [=] { this->actionCallback(input, on_press, false); }); })
                    .detach();
            }
        }
    }
}

void Functions::handleToKeyTap(const int &input, const int eventType) const {
    int key = PRESSED_STATES.find(eventType) != PRESSED_STATES.end() ? (*this->input_to_key_tap).at(input) : (*this->release_to_key_tap).at(input);
    this->handleToKeyTap(input, key, eventType);
}

void Functions::handleToKeyTap(const int &input, const int key, const int eventType) const {
    if ((*this->buttonState).at(input) == eventType) {
        bool on_press = PRESSED_STATES.find(eventType) != PRESSED_STATES.end();
        if (this->actionCallback(input, on_press, true)) {
            std::thread([=] { this->pressThenRelease(key, [=] { this->actionCallback(input, on_press, false); }); })
                .detach();
        }
    }
}

void Functions::handleToKeyHold(const int &input) const {
    int key = (*this->input_to_key_hold).at(input); // no on-release for this one
    this->handleToKeyHold(input, key);
}

void Functions::handleToKeyHold(const int &input, const int key) const {
    int input_state = (*this->buttonState).at(input);
    if (input_state == JUST_PRESSED && this->actionCallback(input, true, true)) {
        this->sendInput(key, KEYEVENTF_SCANCODE);
        this->actionCallback(input, true, false);
    } else if (input_state == JUST_RELEASED && this->actionCallback(input, false, true)) {
        this->sendInput(key, KEYEVENTF_KEYUP);
        this->actionCallback(input, false, false);
    }
}

void Functions::handleState(int &state, bool is_pressed) const {
    if (is_pressed) {
        if (state == RELEASED) {
            state = JUST_PRESSED;
        }
    } else {
        if (state == PRESSED) {
            state = JUST_RELEASED;
        }
    }
}

bool Functions::isBufferFree(const int second_input_delay_mills, const int subsequent_inputs_delay_millis, const int &buttonState, BufferState &bufferState) const {
    auto now = std::chrono::steady_clock::now();
    if (now - bufferState.last_executed <= std::chrono::milliseconds(subsequent_inputs_delay_millis)) {
        return false;
    } else if (!bufferState.is_unleashed && now - bufferState.last_pressed <= std::chrono::milliseconds(second_input_delay_mills)) {
        return false;
    } else {
        bufferState.last_executed = now;
        bufferState.is_unleashed = (buttonState == PRESSED);
        if (buttonState == JUST_PRESSED) {
            bufferState.last_pressed = now;
        }
        return true;
    }
}
