#include <fstream>
#include <iostream>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <thread>
#include <windows.h>
#include "swarm.h"
#include "funcs.h"

namespace swarm
{
    void run(
        std::unordered_map<std::string, std::string> &buttonState,
        bool hasTriggers,
        Json::Value &config,
        int screenWidth,
        int screenHeight,
        SDL_Joystick *joystick)
    {
        int LEFT_JS_X_ID = config["left_joystick_x_id"].asInt();
        int LEFT_JS_Y_ID = config["left_joystick_y_id"].asInt();
        int RIGHT_JS_X_ID = config["right_joystick_x_id"].asInt();
        int RIGHT_JS_Y_ID = config["right_joystick_y_id"].asInt();
        int LEFT_TRIGGER_ID = config["left_trigger_id"].asInt();
        int RIGHT_TRIGGER_ID = config["right_trigger_id"].asInt();
        float LEFT_JS_DEAD_ZONE = config["left_joystick_dead_zone"].asFloat();
        float RIGHT_JS_DEAD_ZONE = config["right_joystick_dead_zone"].asFloat();
        float RIGHT_TRIGGER_DEAD_ZONE = config["right_trigger_dead_zone"].asFloat();
        float LEFT_TRIGGER_DEAD_ZONE = config["left_trigger_dead_zone"].asFloat();
        float RIGHT_JS_SENSITIVITY = config["right_joystick_sensitivity"].asFloat();
        std::unordered_map<int, std::string> buttonMapping;
        for (const auto &configKey : config["button_mapping"].getMemberNames())
        {
            int key = config["button_mapping"][configKey].asInt() - 1;
            buttonMapping[key] = configKey;
        }
        bool running = config["run_automatically"].asBool();
        bool highPrecision;
        std::ifstream configFile("swarm/config.json");
        Json::Value specificConfig;
        if (configFile.is_open())
        {
            configFile >> specificConfig;
            configFile.close();
        }
        else
        {
            std::cerr << "Error opening config file." << std::endl;
            return;
        }
        bool highPrecisionAlwaysOn = specificConfig["high_precision_on_by_default"].asBool();
        float MAX_RADIUS_HIGH_PRECISION_OFF = specificConfig["default_radius"].asFloat();
        float currentRadius = MAX_RADIUS_HIGH_PRECISION_OFF;

        Functions functions;

        int center_x = screenWidth / 2;
        int center_y = screenHeight / 2;

        auto INPUT_TO_KEY_TAP = std::unordered_map<std::string, WORD>{
            {"START", VK_ESCAPE},
            {"X", 'C'},
            {"Y", 'O'}};

        auto INPUT_TO_KEY_HOLD = std::unordered_map<std::string, WORD>{
            {"LEFT_JS_LEFT", 'A'},
            {"LEFT_JS_RIGHT", 'D'},
            {"LEFT_JS_UP", 'W'},
            {"LEFT_JS_DOWN", 'S'},
            {"B", VK_TAB},
            {"R2", 'T'}};

        auto INPUT_TO_MOUSE_MOVE = std::unordered_map<std::string, std::pair<int, int>>{
            {"LEFT", {720, center_y}},
            {"RIGHT", {1200, center_y}},
            {"UP", {center_x, center_y}},
            {"DOWN", {center_x, 825}},
            {"R3", {center_x, center_y}}};

        auto RELEASE_TO_KEY_TAP = std::unordered_map<std::string, WORD>{
            {"R1", 'R'},
            {"L1", 'E'}};

        auto INPUT_TO_LOGIC_BEFORE = std::unordered_map<std::string, std::function<void()>>{
            {"R2", [&]()
             { functions.moveMouse(center_x, center_y); }},
            {"R3", [&]()
             { highPrecisionAlwaysOn = !highPrecisionAlwaysOn; }}};

        auto INPUT_TO_LOGIC_AFTER = std::unordered_map<std::string, std::function<void()>>{
            {"R1", [&]()
             { currentRadius = MAX_RADIUS_HIGH_PRECISION_OFF;; }},
            {"L1", [&]()
             { currentRadius = MAX_RADIUS_HIGH_PRECISION_OFF;; }}};

        functions.setMaps(INPUT_TO_LOGIC_BEFORE, INPUT_TO_LOGIC_AFTER);

        try
        {
            float leftX, leftY, rightX, rightY;
            bool isLeftXActive, isLeftYActive, isRightXActive, isRightYActive, isLeftTriggerAxisActive, isRightTriggerAxisActive;

            auto lastUpdateTime = std::chrono::steady_clock::now();

            while (true)
            {
                auto loopStartTime = std::chrono::steady_clock::now();
                std::vector<SDL_Event> events;
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    events.push_back(event);
                }
                if (!running)
                {
                    for (const auto &event : events)
                    {
                        if (event.type == SDL_JOYBUTTONDOWN)
                        {
                            std::string button = buttonMapping[event.jbutton.button];
                            if (button == "ACTIVATE")
                            {
                                running = true;
                                break;
                            }
                        }
                    }
                }
                else
                {
                    // state
                    for (auto &pair : buttonState)
                    {
                        std::string &state = pair.second;
                        if (state == "JUST_PRESSED")
                        {
                            state = "PRESSED";
                        }
                        else if (state == "JUST_RELEASED")
                        {
                            state = "NOT_PRESSED";
                        }
                    }

                    leftX = SDL_JoystickGetAxis(joystick, LEFT_JS_X_ID) / 32768.0f;
                    leftY = SDL_JoystickGetAxis(joystick, LEFT_JS_Y_ID) / 32768.0f;
                    isLeftXActive = std::abs(leftX) > LEFT_JS_DEAD_ZONE;
                    isLeftYActive = std::abs(leftY) > LEFT_JS_DEAD_ZONE;

                    functions.handleState(&buttonState["LEFT_JS_LEFT"], isLeftXActive && leftX < 0);
                    functions.handleState(&buttonState["LEFT_JS_RIGHT"], isLeftXActive && leftX > 0);
                    functions.handleState(&buttonState["LEFT_JS_UP"], isLeftYActive && leftY < 0);
                    functions.handleState(&buttonState["LEFT_JS_DOWN"], isLeftYActive && leftY > 0);

                    rightX = SDL_JoystickGetAxis(joystick, RIGHT_JS_X_ID) / 32768.0f;
                    rightY = SDL_JoystickGetAxis(joystick, RIGHT_JS_Y_ID) / 32768.0f;
                    isRightXActive = std::abs(rightX) > RIGHT_JS_DEAD_ZONE;
                    isRightYActive = std::abs(rightY) > RIGHT_JS_DEAD_ZONE;

                    if (hasTriggers)
                    {
                        isLeftTriggerAxisActive = (SDL_JoystickGetAxis(joystick, LEFT_TRIGGER_ID) + 32768) / 65536.0f > LEFT_TRIGGER_DEAD_ZONE;
                        isRightTriggerAxisActive = (SDL_JoystickGetAxis(joystick, RIGHT_TRIGGER_ID) + 32768) / 65536.0f > RIGHT_TRIGGER_DEAD_ZONE;
                    }

                    for (const auto &event : events)
                    {
                        if (event.type == SDL_JOYBUTTONDOWN)
                        {
                            std::string button = buttonMapping[event.jbutton.button];
                            functions.handleState(&buttonState[button], true);
                        }
                        else if (event.type == SDL_JOYBUTTONUP)
                        {
                            std::string button = buttonMapping[event.jbutton.button];
                            functions.handleState(&buttonState[button], false);
                        }
                        else if (event.type == SDL_JOYHATMOTION)
                        {
                            functions.handleState(&buttonState["LEFT"], event.jhat.value == SDL_HAT_LEFT);
                            functions.handleState(&buttonState["RIGHT"], event.jhat.value == SDL_HAT_RIGHT);
                            functions.handleState(&buttonState["DOWN"], event.jhat.value == SDL_HAT_DOWN);
                            functions.handleState(&buttonState["UP"], event.jhat.value == SDL_HAT_UP);
                        }
                    }

                    if (buttonState["ACTIVATE"] == "JUST_PRESSED")
                    {
                        running = false;
                        continue;
                    }
                    if (hasTriggers)
                    {
                        highPrecision = isLeftTriggerAxisActive;
                        functions.handleState(&buttonState["R2"], isRightTriggerAxisActive);
                    }
                    else
                    {
                        highPrecision = buttonState["L2"] == "PRESSED" || buttonState["L2"] == "JUST_PRESSED";
                    }
                    if (buttonState["R1"] == "PRESSED" || buttonState["L1"] == "PRESSED")
                    {
                        currentRadius += RIGHT_JS_SENSITIVITY * 0.05f;
                    }
                    if (buttonState["L3"] == "JUST_PRESSED")
                    {
                        buttonState["R1"] = "NOT_PRESSED";
                        buttonState["L1"] = "NOT_PRESSED";
                    }

                    // action
                    for (const auto &[input, keyToTap] : INPUT_TO_KEY_TAP)
                    {
                        functions.handleToKeyTapInput(buttonState[input], input, keyToTap);
                    }
                    for (const auto &[input, keyToPress] : INPUT_TO_KEY_HOLD)
                    {
                        functions.handleToKeyHoldInput(buttonState[input], input, keyToPress);
                    }
                    for (const auto &[input, pos] : INPUT_TO_MOUSE_MOVE)
                    {
                        functions.handleToMouseAbsoluteMoveInput(buttonState[input], input, pos.first, pos.second);
                    }
                    for (const auto &[input, keyToTap] : RELEASE_TO_KEY_TAP)
                    {
                        functions.handleToKeyTapRelease(buttonState[input], input, keyToTap);
                    }
                    functions.handleToClickInput(buttonState["A"], "A", 0);

                    if (isRightXActive || isRightYActive)
                    {
                        if (highPrecisionAlwaysOn || highPrecision)
                        {
                            if (std::chrono::steady_clock::now() - lastUpdateTime > std::chrono::milliseconds(100))
                            {
                                functions.moveMouseRelative(
                                    round(rightX * RIGHT_JS_SENSITIVITY * 500),
                                    round(rightY * RIGHT_JS_SENSITIVITY * 500));
                                lastUpdateTime = std::chrono::steady_clock::now();
                            }
                        }
                        else
                        {
                            functions.moveMouse(
                                round((rightX * center_x * currentRadius) + (center_x)),
                                round((rightY * center_y * currentRadius) + (center_y)));
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::microseconds(std::max(10000 - std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - loopStartTime).count(), 0LL)));
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
    }
}
