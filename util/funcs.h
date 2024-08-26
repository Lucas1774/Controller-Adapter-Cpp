#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <functional>
#include <unordered_map>
#include <string>
#include <SDL2/SDL.h>
#include <windows.h>

class Functions
{
public:
    void setMaps(std::unordered_map<std::string, std::function<void()>> input_to_logic_before,
                 std::unordered_map<std::string, std::function<void()>> input_to_logic_after);
    void sendInput(int key, DWORD flags);
    void moveMouse(int x, int y);
    void moveMouseRelative(int x, int y);
    void pressThenRelease(int key_to_tap, std::function<void()> callback = nullptr);
    void clickAndHoldButton(int button_to_click, std::function<void()> callback = nullptr);
    void releaseButton(int button_to_release, std::function<void()> callback = nullptr);
    void clickThenRelease(int button_to_click, std::function<void()> callback = nullptr);
    void handleToKeyTapInput(const std::string &input_state, const std::string &input, int key_to_tap);
    void handleToKeyHoldInput(const std::string &input_state, const std::string &input, int key_to_press);
    void handleToMouseAbsoluteMoveInput(const std::string &input_state, const std::string &input, int x, int y);
    void handleToClickInput(const std::string &input_state, const std::string &input, int button_to_click);
    void handleToClickAndHoldOrReleaseInput(const std::string &input_state, const std::string &input, int button_to_click_or_release);
    void handleToKeyTapRelease(const std::string &input_state, const std::string &input, int key_to_tap);
    void handleState(std::string *const state, bool is_pressed);

private:
    void callbackBeforeAction(const std::string &input);
    void callbackAfterAction(const std::string &input);
    std::unordered_map<std::string, std::function<void()>> input_to_logic_before;
    std::unordered_map<std::string, std::function<void()>> input_to_logic_after;
    std::unordered_map<int, DWORD> mouse_mapping = {
        {SDL_BUTTON_LEFT, VK_LBUTTON},
        {SDL_BUTTON_RIGHT, VK_RBUTTON},
        {SDL_BUTTON_MIDDLE, VK_MBUTTON}};
    std::unordered_map<int, DWORD> mouse_down_mapping = {
        {SDL_BUTTON_LEFT, MOUSEEVENTF_LEFTDOWN},
        {SDL_BUTTON_RIGHT, MOUSEEVENTF_RIGHTDOWN},
        {SDL_BUTTON_MIDDLE, MOUSEEVENTF_MIDDLEDOWN}};
    std::unordered_map<int, DWORD> mouse_up_mapping = {
        {SDL_BUTTON_LEFT, MOUSEEVENTF_LEFTUP},
        {SDL_BUTTON_RIGHT, MOUSEEVENTF_RIGHTUP},
        {SDL_BUTTON_MIDDLE, MOUSEEVENTF_MIDDLEUP}};
};

#endif // FUNCTIONS_H
