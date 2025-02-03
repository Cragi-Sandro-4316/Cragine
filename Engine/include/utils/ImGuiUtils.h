#pragma once 

#include <imgui.h>
#include "input/Keycodes.h"

ImGuiKey mapCragineKeyToImGuiKey(int cragineKey) {
    switch (cragineKey) {
        case CRAGINE_KEY_SPACE: return ImGuiKey_Space;
        case CRAGINE_KEY_APOSTROPHE: return ImGuiKey_Apostrophe; /* ' */
        case CRAGINE_KEY_COMMA: return ImGuiKey_Comma; /* , */
        case CRAGINE_KEY_MINUS: return ImGuiKey_Minus; /* - */
        case CRAGINE_KEY_PERIOD: return ImGuiKey_Period; /* . */
        case CRAGINE_KEY_SLASH: return ImGuiKey_Slash; /* / */
        case CRAGINE_KEY_0: return ImGuiKey_0;
        case CRAGINE_KEY_1: return ImGuiKey_1;
        case CRAGINE_KEY_2: return ImGuiKey_2;
        case CRAGINE_KEY_3: return ImGuiKey_3;
        case CRAGINE_KEY_4: return ImGuiKey_4;
        case CRAGINE_KEY_5: return ImGuiKey_5;
        case CRAGINE_KEY_6: return ImGuiKey_6;
        case CRAGINE_KEY_7: return ImGuiKey_7;
        case CRAGINE_KEY_8: return ImGuiKey_8;
        case CRAGINE_KEY_9: return ImGuiKey_9;
        case CRAGINE_KEY_SEMICOLON: return ImGuiKey_Semicolon; /* ; */
        case CRAGINE_KEY_EQUAL: return ImGuiKey_Equal; /* = */
        case CRAGINE_KEY_A: return ImGuiKey_A;
        case CRAGINE_KEY_B: return ImGuiKey_B;
        case CRAGINE_KEY_C: return ImGuiKey_C;
        case CRAGINE_KEY_D: return ImGuiKey_D;
        case CRAGINE_KEY_E: return ImGuiKey_E;
        case CRAGINE_KEY_F: return ImGuiKey_F;
        case CRAGINE_KEY_G: return ImGuiKey_G;
        case CRAGINE_KEY_H: return ImGuiKey_H;
        case CRAGINE_KEY_I: return ImGuiKey_I;
        case CRAGINE_KEY_J: return ImGuiKey_J;
        case CRAGINE_KEY_K: return ImGuiKey_K;
        case CRAGINE_KEY_L: return ImGuiKey_L;
        case CRAGINE_KEY_M: return ImGuiKey_M;
        case CRAGINE_KEY_N: return ImGuiKey_N;
        case CRAGINE_KEY_O: return ImGuiKey_O;
        case CRAGINE_KEY_P: return ImGuiKey_P;
        case CRAGINE_KEY_Q: return ImGuiKey_Q;
        case CRAGINE_KEY_R: return ImGuiKey_R;
        case CRAGINE_KEY_S: return ImGuiKey_S;
        case CRAGINE_KEY_T: return ImGuiKey_T;
        case CRAGINE_KEY_U: return ImGuiKey_U;
        case CRAGINE_KEY_V: return ImGuiKey_V;
        case CRAGINE_KEY_W: return ImGuiKey_W;
        case CRAGINE_KEY_X: return ImGuiKey_X;
        case CRAGINE_KEY_Y: return ImGuiKey_Y;
        case CRAGINE_KEY_Z: return ImGuiKey_Z;
        case CRAGINE_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket; /* [ */
        case CRAGINE_KEY_BACKSLASH: return ImGuiKey_Backslash; /* \ */
        case CRAGINE_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket; /* ] */
        case CRAGINE_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent; /* ` */
        case CRAGINE_KEY_ESCAPE: return ImGuiKey_Escape;
        case CRAGINE_KEY_ENTER: return ImGuiKey_Enter;
        case CRAGINE_KEY_TAB: return ImGuiKey_Tab;
        case CRAGINE_KEY_BACKSPACE: return ImGuiKey_Backspace;
        case CRAGINE_KEY_INSERT: return ImGuiKey_Insert;
        case CRAGINE_KEY_DELETE: return ImGuiKey_Delete;
        case CRAGINE_KEY_RIGHT: return ImGuiKey_RightArrow;
        case CRAGINE_KEY_LEFT: return ImGuiKey_LeftArrow;
        case CRAGINE_KEY_DOWN: return ImGuiKey_DownArrow;
        case CRAGINE_KEY_UP: return ImGuiKey_UpArrow;
        case CRAGINE_KEY_PAGE_UP: return ImGuiKey_PageUp;
        case CRAGINE_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
        case CRAGINE_KEY_HOME: return ImGuiKey_Home;
        case CRAGINE_KEY_END: return ImGuiKey_End;
        case CRAGINE_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
        case CRAGINE_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case CRAGINE_KEY_NUM_LOCK: return ImGuiKey_NumLock;
        case CRAGINE_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case CRAGINE_KEY_PAUSE: return ImGuiKey_Pause;
        case CRAGINE_KEY_F1: return ImGuiKey_F1;
        case CRAGINE_KEY_F2: return ImGuiKey_F2;
        case CRAGINE_KEY_F3: return ImGuiKey_F3;
        case CRAGINE_KEY_F4: return ImGuiKey_F4;
        case CRAGINE_KEY_F5: return ImGuiKey_F5;
        case CRAGINE_KEY_F6: return ImGuiKey_F6;
        case CRAGINE_KEY_F7: return ImGuiKey_F7;
        case CRAGINE_KEY_F8: return ImGuiKey_F8;
        case CRAGINE_KEY_F9: return ImGuiKey_F9;
        case CRAGINE_KEY_F10: return ImGuiKey_F10;
        case CRAGINE_KEY_F11: return ImGuiKey_F11;
        case CRAGINE_KEY_F12: return ImGuiKey_F12;
        case CRAGINE_KEY_KP_0: return ImGuiKey_Keypad0;
        case CRAGINE_KEY_KP_1: return ImGuiKey_Keypad1;
        case CRAGINE_KEY_KP_2: return ImGuiKey_Keypad2;
        case CRAGINE_KEY_KP_3: return ImGuiKey_Keypad3;
        case CRAGINE_KEY_KP_4: return ImGuiKey_Keypad4;
        case CRAGINE_KEY_KP_5: return ImGuiKey_Keypad5;
        case CRAGINE_KEY_KP_6: return ImGuiKey_Keypad6;
        case CRAGINE_KEY_KP_7: return ImGuiKey_Keypad7;
        case CRAGINE_KEY_KP_8: return ImGuiKey_Keypad8;
        case CRAGINE_KEY_KP_9: return ImGuiKey_Keypad9;
        case CRAGINE_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
        case CRAGINE_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case CRAGINE_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case CRAGINE_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case CRAGINE_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
        case CRAGINE_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
        case CRAGINE_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
        case CRAGINE_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
        case CRAGINE_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case CRAGINE_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
        case CRAGINE_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
        case CRAGINE_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
        case CRAGINE_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case CRAGINE_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
        case CRAGINE_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
        case CRAGINE_KEY_MENU: return ImGuiKey_Menu;
        default: return ImGuiKey_None;
    }
}