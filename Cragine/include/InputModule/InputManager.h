#pragma once

#include "InputModule/KeyCode.h"
#include "utils/Logger.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>

namespace crg {

    class InputModule;

    class InputManager {
    public:
        InputManager(GLFWwindow* window)
        : m_window(window) {
            LOG_CORE_INFO("Creating InputManager. {}", (void*) this);

            KeyCode allKeys[] = {
                KeyCode::KeySpace,KeyCode::KeyApostrophe,KeyCode::KeyComma,KeyCode::KeyMinus,KeyCode::KeyPeriod,KeyCode::KeySlash,KeyCode::KeyD0,
                KeyCode::KeyD1,KeyCode::KeyD2,KeyCode::KeyD3,KeyCode::KeyD4,KeyCode::KeyD5,KeyCode::KeyD6,KeyCode::KeyD7,
                KeyCode::KeyD8,KeyCode::KeyD9,KeyCode::KeySemicolon,KeyCode::KeyEqual,KeyCode::KeyA,KeyCode::KeyB,
                KeyCode::KeyC,KeyCode::KeyD,KeyCode::KeyE,KeyCode::KeyF,KeyCode::KeyG,KeyCode::KeyH,KeyCode::KeyI,
                KeyCode::KeyJ,KeyCode::KeyK,KeyCode::KeyL,KeyCode::KeyM,KeyCode::KeyN,KeyCode::KeyO,KeyCode::KeyP,
                KeyCode::KeyQ,KeyCode::KeyR,KeyCode::KeyS,KeyCode::KeyT,KeyCode::KeyU,KeyCode::KeyV,KeyCode::KeyW,
                KeyCode::KeyX,KeyCode::KeyY,KeyCode::KeyZ,KeyCode::KeyLeftBracket,KeyCode::KeyBackslash,KeyCode::KeyRightBracket,
                KeyCode::KeyGraveAccent,KeyCode::KeyWorld1,KeyCode::KeyWorld2,KeyCode::KeyEscape,KeyCode::KeyEnter,KeyCode::KeyTab,
                KeyCode::KeyBackspace,KeyCode::KeyInsert,KeyCode::KeyDelete,KeyCode::KeyRight,KeyCode::KeyLeft,KeyCode::KeyDown,
                KeyCode::KeyUp,KeyCode::KeyPageUp,KeyCode::KeyPageDown,KeyCode::KeyHome,KeyCode::KeyEnd,KeyCode::KeyCapsLock,
                KeyCode::KeyScrollLock,KeyCode::KeyNumLock,KeyCode::KeyPrintScreen,KeyCode::KeyPause,KeyCode::KeyF1,KeyCode::KeyF2,
                KeyCode::KeyF3,KeyCode::KeyF4,KeyCode::KeyF5,KeyCode::KeyF6,KeyCode::KeyF7,KeyCode::KeyF8,KeyCode::KeyF9,KeyCode::KeyF10,
                KeyCode::KeyF11,KeyCode::KeyF12,KeyCode::KeyF13,KeyCode::KeyF14,KeyCode::KeyF15,KeyCode::KeyF16,KeyCode::KeyF17,KeyCode::KeyF18,
                KeyCode::KeyF19,KeyCode::KeyF20,KeyCode::KeyF21,KeyCode::KeyF22,KeyCode::KeyF23,KeyCode::KeyF24,KeyCode::KeyF25,KeyCode::KeyKP0,
                KeyCode::KeyKP1,KeyCode::KeyKP2,KeyCode::KeyKP3,KeyCode::KeyKP4,KeyCode::KeyKP5,KeyCode::KeyKP6,KeyCode::KeyKP7,KeyCode::KeyKP8,
                KeyCode::KeyKP9,KeyCode::KeyKPDecimal,KeyCode::KeyKPDivide,KeyCode::KeyKPMultiply,KeyCode::KeyKPSubtract,KeyCode::KeyKPAdd,
                KeyCode::KeyKPEnter,KeyCode::KeyKPEqual,KeyCode::KeyLeftShift,KeyCode::KeyLeftControl,KeyCode::KeyLeftAlt,KeyCode::KeyLeftSuper,
                KeyCode::KeyRightShift,KeyCode::KeyRightControl,KeyCode::KeyRightAlt,KeyCode::KeyRightSuper,KeyCode::KeyMenu,KeyCode::KeyLast,
            };

            size_t NUM_KEYS = sizeof(allKeys) / sizeof(allKeys[0]);

            for (size_t i = 0; i < NUM_KEYS; i++) {
                m_keyIndices[allKeys[i]] = i;
            }

            m_keyStates[0].resize(NUM_KEYS, false);
            m_keyStates[1].resize(NUM_KEYS, false);
        }


        InputManager() {
            LOG_CORE_INFO("Creating InputManager. {}", (void*) this);

            m_window = nullptr;
        }

        ~InputManager() {
            LOG_CORE_INFO("Destroying InputManager. {}", (void*) this);
        }

        InputManager(const InputManager & other) = delete;

        bool keyPressed(KeyCode keycode) const {
            auto index = m_keyIndices[keycode];
            return m_keyStates[m_current][index];
        }

        bool keyJustPressed(KeyCode keycode) const {
            auto index = m_keyIndices[keycode];
            return m_keyStates[m_current][index] && !m_keyStates[!m_current][index];
        }


    private:

        bool getKeyState(KeyCode keycode) {
            auto state = glfwGetKey(m_window, (int)keycode);

            return state == GLFW_PRESS;
        }


        void update() {
            m_current = !m_current;

            if(!m_window) {
                LOG_CORE_ERROR("InputManager: window is nullptr");
            }

            for (auto& [keycode, index] : m_keyIndices) {
                m_keyStates[m_current][index] = getKeyState(keycode);

            }


        }


    private:

        mutable std::unordered_map<KeyCode, size_t> m_keyIndices;

        mutable std::vector<bool> m_keyStates[2];

        bool m_current = true;

        GLFWwindow* m_window;

        friend class InputModule;
    };

}
