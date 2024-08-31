#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "constants.h"
#include <SDL2/SDL.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <windows.h>

class Functions {
  public:
    void setMaps(const std::unordered_map<int, int> *buttonState,
                 const std::unordered_map<int, std::pair<int, int> *> *input_to_mouse_move,
                 const std::unordered_map<int, std::pair<int, int> *> *release_to_mouse_move,
                 const std::unordered_map<int, int> *input_to_mouse_click,
                 const std::unordered_map<int, int> *release_to_mouse_click,
                 const std::unordered_map<int, int> *input_to_button_toggle,
                 const std::unordered_map<int, int> *release_to_button_toggle,
                 const std::unordered_map<int, WORD> *input_to_key_tap,
                 const std::unordered_map<int, WORD> *release_to_key_tap,
                 const std::unordered_map<int, WORD> *input_to_key_hold,
                 const std::unordered_map<int, std::function<void()>> *input_to_logic_before,
                 const std::unordered_map<int, std::function<void()>> *input_to_logic_after,
                 const std::unordered_map<int, std::function<void()>> *release_to_logic_before,
                 const std::unordered_map<int, std::function<void()>> *release_to_logic_after);
    void moveMouse(const int x, const int y);
    void moveMouseRelative(const int x, const int y);
    void click(const int button, const std::function<void()> callback = nullptr);
    void handleToMouseAbsoluteMove(const int &input, const int eventType);
    void handleToClick(const int &input, const int eventType);
    void handleToClick(const int &input, const int button, const int eventType);
    void handleToButtonToggle(const int &input, const int eventType);
    void handleToButtonToggle(const int &input, const int button, const int eventType);
    void handleToKeyTap(const int &input, const int eventType);
    void handleToKeyTap(const int &input, const int key, const int eventType);
    void handleToKeyHold(const int &input);
    void handleToKeyHold(const int &input, const int key);
    void handleState(int &state, const bool is_pressed);

  private:
    const std::unordered_set<int> PRESSED_STATES = {PRESSED, JUST_PRESSED};
    const std::unordered_set<int> RELEASED_STATES = {JUST_RELEASED, RELEASED};
    const std::unordered_map<int, DWORD> BUTTON_ID_TO_PRESS_EVENT = {
        {SDL_BUTTON_LEFT, MOUSEEVENTF_LEFTDOWN},
        {SDL_BUTTON_RIGHT, MOUSEEVENTF_RIGHTDOWN},
        {SDL_BUTTON_MIDDLE, MOUSEEVENTF_MIDDLEDOWN}};
    const std::unordered_map<int, DWORD> BUTOTN_ID_TO_RELEASE_EVENT = {
        {SDL_BUTTON_LEFT, MOUSEEVENTF_LEFTUP},
        {SDL_BUTTON_RIGHT, MOUSEEVENTF_RIGHTUP},
        {SDL_BUTTON_MIDDLE, MOUSEEVENTF_MIDDLEUP}};
    void sendInput(const int key, const DWORD flags);
    void actionCallback(const int &input, const bool on_press, const bool before);
    void pressButton(const int button_to_click, const std::function<void()> callback = nullptr);
    void releaseButton(const int button_to_release, const std::function<void()> callback = nullptr);
    void pressThenRelease(const int key_to_tap, const std::function<void()> callback = nullptr);
    void clickThenRelease(const int button_to_click, const std::function<void()> callback = nullptr);
    const std::unordered_map<int, int> *buttonState;
    const std::unordered_map<int, std::pair<int, int> *> *input_to_mouse_move;
    const std::unordered_map<int, std::pair<int, int> *> *release_to_mouse_move;
    const std::unordered_map<int, int> *input_to_mouse_click;
    const std::unordered_map<int, int> *release_to_mouse_click;
    const std::unordered_map<int, int> *input_to_button_toggle;
    const std::unordered_map<int, int> *release_to_button_toggle;
    const std::unordered_map<int, WORD> *input_to_key_tap;
    const std::unordered_map<int, WORD> *release_to_key_tap;
    const std::unordered_map<int, WORD> *input_to_key_hold;
    const std::unordered_map<int, std::function<void()>> *input_to_logic_before;
    const std::unordered_map<int, std::function<void()>> *input_to_logic_after;
    const std::unordered_map<int, std::function<void()>> *release_to_logic_before;
    const std::unordered_map<int, std::function<void()>> *release_to_logic_after;
};

#endif // FUNCTIONS_H
