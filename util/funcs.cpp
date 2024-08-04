#include <functional>
#include <thread>
#include <chrono>
#include <windows.h>
#include "funcs.h"

void Functions::setMaps(std::unordered_map<std::string, std::function<void()>> input_to_logic_before,
                        std::unordered_map<std::string, std::function<void()>> input_to_logic_after)
{
    this->input_to_logic_before = input_to_logic_before;
    this->input_to_logic_after = input_to_logic_after;
}

void Functions::sendInput(int key, DWORD flags)
{
    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC);
    ip.ki.dwFlags = flags;
    SendInput(1, &ip, sizeof(INPUT));
}

void Functions::callbackBeforeAction(const std::string &input)
{
    auto inputToLogic = input_to_logic_before.find(input);
    if (inputToLogic != input_to_logic_before.end())
    {
        inputToLogic->second();
    }
}

void Functions::callbackAfterAction(const std::string &input)
{
    auto inputToLogic = input_to_logic_after.find(input);
    if (inputToLogic != input_to_logic_after.end())
    {
        inputToLogic->second();
    }
}

void Functions::moveMouse(int x, int y)
{
    SetCursorPos(x, y);
}

void Functions::moveMouseRelative(int x, int y)
{
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x + x, p.y + y);
}

void Functions::pressThenRelease(int key_to_tap, std::function<void()> callback)
{

    this->sendInput(key_to_tap, KEYEVENTF_SCANCODE);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    this->sendInput(key_to_tap, KEYEVENTF_KEYUP);
    if (callback)
    {
        callback();
    }
}

void Functions::clickThenRelease(int button_to_click, std::function<void()> callback)
{
    INPUT ip[2] = {0};
    ip[0].type = INPUT_MOUSE;
    ip[0].mi.dwFlags = (button_to_click == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    ip[1].type = INPUT_MOUSE;
    ip[1].mi.dwFlags = (button_to_click == 0) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
    SendInput(2, ip, sizeof(INPUT));
    if (callback)
    {
        callback();
    }
}

void Functions::handleToKeyTapInput(const std::string &input_state, const std::string &input, int key_to_tap)
{
    if (input_state == "JUST_PRESSED")
    {
        callbackBeforeAction(input);
        std::thread([=]
                    { pressThenRelease(key_to_tap, [=]
                                       { callbackAfterAction(input); }); })
            .detach();
    }
}

void Functions::handleToKeyHoldInput(const std::string &input_state, const std::string &input, int key_to_press)
{
    if (input_state == "JUST_PRESSED")
    {
        callbackBeforeAction(input);
        this->sendInput(key_to_press, KEYEVENTF_SCANCODE);
        callbackAfterAction(input);
    }
    else if (input_state == "JUST_RELEASED")
    {
        callbackBeforeAction(input);
        this->sendInput(key_to_press, KEYEVENTF_KEYUP);
        callbackAfterAction(input);
    }
}

void Functions::handleToMouseAbsoluteMoveInput(const std::string &input_state, const std::string &input, int x, int y)
{
    if (input_state == "JUST_PRESSED")
    {
        callbackBeforeAction(input);
        moveMouse(x, y);
        callbackAfterAction(input);
    }
}

void Functions::handleToClickInput(const std::string &input_state, const std::string &input, int button_to_click)
{
    if (input_state == "JUST_PRESSED")
    {
        callbackBeforeAction(input);
        std::thread([=]
                    { clickThenRelease(button_to_click, [=]
                                       { callbackAfterAction(input); }); })
            .detach();
    }
}

// Function to handle key tap release input
void Functions::handleToKeyTapRelease(const std::string &input_state, const std::string &input, int key_to_tap)
{
    if (input_state == "JUST_RELEASED")
    {
        callbackBeforeAction(input);
        std::thread([=]
                    { pressThenRelease(key_to_tap, [=]
                                       { callbackAfterAction(input); }); })
            .detach();
    }
}

void Functions::handleState(std::string *const state, bool is_pressed)
{
    if (is_pressed)
    {
        if (*state == "NOT_PRESSED")
        {
            *state = "JUST_PRESSED";
        }
    }
    else
    {
        if (*state == "PRESSED")
        {
            *state = "JUST_RELEASED";
        }
    }
}
