#include "funcs.h"
#include <chrono>
#include <functional>
#include <thread>
#include <windows.h>

void Functions::setMaps(std::unordered_map<std::string, std::string> *buttonState,
                        std::unordered_map<std::string, WORD> *input_to_key_tap,
                        std::unordered_map<std::string, WORD> *input_to_key_hold,
                        std::unordered_map<std::string, WORD> *release_to_key_tap,
                        std::unordered_map<std::string, std::pair<int, int> *> *input_to_mouse_move,
                        std::unordered_map<std::string, int> *input_to_mouse_click,
                        std::unordered_map<std::string, int> *input_to_mouse_button_hold_or_release,
                        std::unordered_map<std::string, std::pair<int, int>> *release_to_mouse_move,
                        std::unordered_map<std::string, std::function<void()>> *input_to_logic_before,
                        std::unordered_map<std::string, std::function<void()>> *input_to_logic_after,
                        std::unordered_map<std::string, std::function<void()>> *release_to_logic_before,
                        std::unordered_map<std::string, std::function<void()>> *release_to_logic_after) {
    this->buttonState = buttonState;
    this->input_to_key_tap = input_to_key_tap;
    this->input_to_key_hold = input_to_key_hold;
    this->release_to_key_tap = release_to_key_tap;
    this->input_to_mouse_move = input_to_mouse_move;
    this->input_to_mouse_click = input_to_mouse_click;
    this->input_to_mouse_button_hold_or_release = input_to_mouse_button_hold_or_release;
    this->release_to_mouse_move = release_to_mouse_move;
    this->input_to_logic_before = input_to_logic_before;
    this->input_to_logic_after = input_to_logic_after;
    this->release_to_logic_before = release_to_logic_before;
    this->release_to_logic_after = release_to_logic_after;
}

void Functions::sendInput(int key, DWORD flags) {
    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC);
    ip.ki.dwFlags = flags;
    SendInput(1, &ip, sizeof(INPUT));
}

void Functions::actionCallback(const std::string &input, const bool on_press, const bool before) {
    const auto *action_map = before
                                 ? (on_press ? input_to_logic_before : release_to_logic_before)
                                 : (on_press ? input_to_logic_after : release_to_logic_after);

    if (action_map != nullptr) {
        auto action = action_map->find(input);
        if (action != action_map->end()) {
            action->second();
        }
    }
}

void Functions::moveMouse(int x, int y) {
    SetCursorPos(x, y);
}

void Functions::moveMouseRelative(int x, int y) {
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x + x, p.y + y);
}

void Functions::pressThenRelease(int key_to_tap, std::function<void()> callback) {
    this->sendInput(key_to_tap, KEYEVENTF_SCANCODE);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    this->sendInput(key_to_tap, KEYEVENTF_KEYUP);
    if (callback) {
        callback();
    }
}

void Functions::clickAndHoldButton(int button_to_click, std::function<void()> callback) {
    INPUT ip = {0};
    ip.type = INPUT_MOUSE;
    ip.mi.dwFlags = mouse_down_mapping[button_to_click];
    SendInput(1, &ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::releaseButton(int button_to_release, std::function<void()> callback) {
    INPUT ip = {0};
    ip.type = INPUT_MOUSE;
    ip.mi.dwFlags = mouse_up_mapping[button_to_release];
    SendInput(1, &ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::clickThenRelease(int button_to_click, std::function<void()> callback) {
    INPUT ip[2] = {0};
    ip[0].type = INPUT_MOUSE;
    ip[0].mi.dwFlags = mouse_down_mapping[button_to_click];
    ip[1].type = INPUT_MOUSE;
    ip[1].mi.dwFlags = mouse_up_mapping[button_to_click];
    SendInput(2, ip, sizeof(INPUT));
    if (callback) {
        callback();
    }
}

void Functions::handleToKeyTapInput(const std::string &input) {
    if ((*buttonState)[input] == "JUST_PRESSED") {
        actionCallback(input, true, true);
        int key_to_tap = (*input_to_key_tap)[input];
        std::thread([=] { pressThenRelease(key_to_tap, [=] { actionCallback(input, true, false); }); })
            .detach();
    }
}

void Functions::handleToKeyHoldInput(const std::string &input) {
    std::string input_state = (*buttonState)[input];
    if (input_state == "JUST_PRESSED") {
        actionCallback(input, true, true);
        int key_to_hold = (*input_to_key_hold)[input];
        this->sendInput(key_to_hold, KEYEVENTF_SCANCODE);
        actionCallback(input, true, false);
    } else if (input_state == "JUST_RELEASED") {
        actionCallback(input, false, true);
        int key_to_hold = (*release_to_key_tap)[input];
        this->sendInput(key_to_hold, KEYEVENTF_KEYUP);
        actionCallback(input, false, false);
    }
}

void Functions::handleToMouseAbsoluteMoveInput(const std::string &input) {
    if ((*buttonState)[input] == "JUST_PRESSED") {
        actionCallback(input, true, true);
        moveMouse((*input_to_mouse_move)[input]->first, (*input_to_mouse_move)[input]->second);
        actionCallback(input, true, false);
    }
}

void Functions::handleToClickInput(const std::string &input) {
    if ((*buttonState)[input] == "JUST_PRESSED") {
        actionCallback(input, true, true);
        int button_to_click = (*input_to_mouse_click)[input];
        std::thread([=] { clickThenRelease(button_to_click, [=] { actionCallback(input, true, false); }); })
            .detach();
    }
}

void Functions::handleToClickAndHoldOrReleaseInput(const std::string &input) {
    if ((*buttonState)[input] == "JUST_PRESSED") {
        actionCallback(input, true, true);
        int button_to_click_or_release = (*input_to_mouse_button_hold_or_release)[input];
        if (!(GetAsyncKeyState(button_to_click_or_release) & 0x8000)) {
            std::thread([=] { clickAndHoldButton(button_to_click_or_release, [=] { actionCallback(input, true, false); }); })
                .detach();
        } else {
            std::thread([=] { releaseButton(button_to_click_or_release, [=] { actionCallback(input, true, false); }); })
                .detach();
        }
    }
}

void Functions::handleToMouseMoveRelease(const std::string &input) {
    if ((*buttonState)[input] == "JUST_RELEASED") {
        actionCallback(input, false, true);
        moveMouse((*release_to_mouse_move)[input].first, (*release_to_mouse_move)[input].second);
        actionCallback(input, false, false);
    }
}

void Functions::handleToKeyTapRelease(const std::string &input) {
    if ((*buttonState)[input] == "JUST_RELEASED") {
        actionCallback(input, false, true);
        int key_to_tap = (*release_to_key_tap)[input];
        std::thread([=] { pressThenRelease(key_to_tap, [=] { actionCallback(input, false, false); }); })
            .detach();
    }
}

void Functions::handleState(std::string *const state, bool is_pressed) {
    if (is_pressed) {
        if (*state == "NOT_PRESSED") {
            *state = "JUST_PRESSED";
        }
    } else {
        if (*state == "PRESSED") {
            *state = "JUST_RELEASED";
        }
    }
}
