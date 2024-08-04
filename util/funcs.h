#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <functional>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <string>
#include <SDL2/SDL.h>

class Functions
{
public:
    void setMaps(std::unordered_map<std::string, std::function<void()>> input_to_logic_before,
              std::unordered_map<std::string, std::function<void()>> input_to_logic_after);
    void moveMouse(int x, int y);
    void moveMouseRelative(int x, int y);
    void pressThenRelease(int key_to_tap, std::function<void()> callback = nullptr);
    void clickThenRelease(int button_to_click, std::function<void()> callback = nullptr);
    void handleToKeyTapInput(const std::string &input_state, const std::string &input, int key_to_tap);
    void handleToKeyHoldInput(const std::string &input_state, const std::string &input, int key_to_press);
    void handleToMouseAbsoluteMoveInput(const std::string &input_state, const std::string &input, int x, int y);
    void handleToClickInput(const std::string &input_state, const std::string &input, int button_to_click);
    void handleToKeyTapRelease(const std::string &input_state, const std::string &input, int key_to_tap);
    void handleState(std::string* const state, bool is_pressed);

private:
    void callbackBeforeAction(const std::string &input);
    void callbackAfterAction(const std::string &input);
    std::unordered_map<std::string, std::function<void()>> input_to_logic_before;
    std::unordered_map<std::string, std::function<void()>> input_to_logic_after;
    std::unordered_map<std::string, int> key_mapping = {
        {"esc", SDLK_ESCAPE},
        {"tab", SDLK_TAB}};
};

#endif // FUNCTIONS_H
