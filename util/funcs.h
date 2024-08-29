#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <SDL2/SDL.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <windows.h>

class Functions {
  public:
    void setMaps(std::unordered_map<std::string, std::string> *buttonState,
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
                 std::unordered_map<std::string, std::function<void()>> *release_to_logic_after);
    void moveMouse(int x, int y);
    void moveMouseRelative(int x, int y);
    void handleToKeyTapInput(const std::string &input);
    void handleToKeyHoldInput(const std::string &input);
    void handleToMouseAbsoluteMoveInput(const std::string &input);
    void handleToClickInput(const std::string &input);
    void handleToClickAndHoldOrReleaseInput(const std::string &input);
    void handleToMouseMoveRelease(const std::string &input);
    void handleToKeyTapRelease(const std::string &input);
    void handleState(std::string *const state, bool is_pressed);

  private:
    void pressThenRelease(int key_to_tap, std::function<void()> callback = nullptr);
    void clickAndHoldButton(int button_to_click, std::function<void()> callback = nullptr);
    void releaseButton(int button_to_release, std::function<void()> callback = nullptr);
    void clickThenRelease(int button_to_click, std::function<void()> callback = nullptr);
    void sendInput(int key, DWORD flags);
    void actionCallback(const std::string &input, const bool on_press, const bool before);
    std::unordered_map<std::string, std::string> *buttonState;
    std::unordered_map<std::string, WORD> *input_to_key_tap;
    std::unordered_map<std::string, WORD> *input_to_key_hold;
    std::unordered_map<std::string, WORD> *release_to_key_tap;
    std::unordered_map<std::string, std::pair<int, int> *> *input_to_mouse_move;
    std::unordered_map<std::string, int> *input_to_mouse_click;
    std::unordered_map<std::string, int> *input_to_mouse_button_hold_or_release;
    std::unordered_map<std::string, std::pair<int, int>> *release_to_mouse_move;
    std::unordered_map<std::string, std::function<void()>> *input_to_logic_before;
    std::unordered_map<std::string, std::function<void()>> *input_to_logic_after;
    std::unordered_map<std::string, std::function<void()>> *release_to_logic_before;
    std::unordered_map<std::string, std::function<void()>> *release_to_logic_after;
    std::unordered_map<int, DWORD>
        mouse_mapping = {
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
