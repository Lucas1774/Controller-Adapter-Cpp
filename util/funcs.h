#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "constants.h"
#include <SDL2/SDL.h>
#include <chrono>
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
                 const std::unordered_map<int, std::function<bool()>> *input_to_logic_before,
                 const std::unordered_map<int, std::function<bool()>> *input_to_logic_after,
                 const std::unordered_map<int, std::function<bool()>> *release_to_logic_before,
                 const std::unordered_map<int, std::function<bool()>> *release_to_logic_after);
    void moveMouse(const int x, const int y) const;
    void moveMouseRelative(const int x, const int y) const;
    void click(const int button, const std::function<void()> callback = nullptr) const;
    void handleToMouseAbsoluteMove(const int &input, const int eventType) const;
    void handleToClick(const int &input, const int eventType) const;
    void handleToClick(const int &input, const int button, const int eventType) const;
    void handleToButtonToggle(const int &input, const int eventType) const;
    void handleToButtonToggle(const int &input, const int button, const int eventType) const;
    void handleToKeyTap(const int &input, const int eventType) const;
    void handleToKeyTap(const int &input, const int key, const int eventType) const;
    void handleToKeyHold(const int &input) const;
    void handleToKeyHold(const int &input, const int key) const;
    void handleState(int &state, const bool is_pressed) const;
    bool isBufferFree(const int second_input_delay_mills,
                      const int subsequent_inputs_delay_millis, const int &buttonState,
                      std::chrono::steady_clock::time_point &last_pressed,
                      std::chrono::steady_clock::time_point &last_executed,
                      bool &is_unleashed) const;

  private:
    const std::unordered_map<int, DWORD> BUTTON_ID_TO_PRESS_EVENT = {
        {SDL_BUTTON_LEFT, MOUSEEVENTF_LEFTDOWN},
        {SDL_BUTTON_RIGHT, MOUSEEVENTF_RIGHTDOWN},
        {SDL_BUTTON_MIDDLE, MOUSEEVENTF_MIDDLEDOWN}};
    const std::unordered_map<int, DWORD> BUTOTN_ID_TO_RELEASE_EVENT = {
        {SDL_BUTTON_LEFT, MOUSEEVENTF_LEFTUP},
        {SDL_BUTTON_RIGHT, MOUSEEVENTF_RIGHTUP},
        {SDL_BUTTON_MIDDLE, MOUSEEVENTF_MIDDLEUP}};
    void sendInput(const int key, const DWORD flags) const;
    bool actionCallback(const int &input, const bool on_press, const bool before) const;
    void pressButton(const int button_to_click, const std::function<void()> callback = nullptr) const;
    void releaseButton(const int button_to_release, const std::function<void()> callback = nullptr) const;
    void pressThenRelease(const int key_to_tap, const std::function<void()> callback = nullptr) const;
    void clickThenRelease(const int button_to_click, const std::function<void()> callback = nullptr) const;
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
    const std::unordered_map<int, std::function<bool()>> *input_to_logic_before;
    const std::unordered_map<int, std::function<bool()>> *input_to_logic_after;
    const std::unordered_map<int, std::function<bool()>> *release_to_logic_before;
    const std::unordered_map<int, std::function<bool()>> *release_to_logic_after;
};

#endif // FUNCTIONS_H
