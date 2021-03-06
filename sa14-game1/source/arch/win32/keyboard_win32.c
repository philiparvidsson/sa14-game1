#ifdef WIN32

/*------------------------------------------------
 * INCLUDES
 *----------------------------------------------*/

#include "base/common.h"
#include "base/debug.h"
#include "input/keyboard.h"

#include <stdint.h>

#include <windows.h>

/*------------------------------------------------
 * GLOBALS
 *----------------------------------------------*/

static keyboardStateT keyboard_state;

/*------------------------------------------------
 * FUNCTIOMS
 *----------------------------------------------*/

void updateKeyboardState(void) {
    BYTE keys[KeyboardNumKeys];
    assert(GetKeyboardState(keys));

    uint8_t map_from[KeyboardNumKeys] = {
        VK_UP,
        VK_DOWN,
        VK_LEFT,
        VK_RIGHT,
        'Q', 'W', 'A', 'S', 'D'
    };

    uint8_t map_to[KeyboardNumKeys] = {
        ArrowUp,
        ArrowDown,
        ArrowLeft,
        ArrowRight,
        'q', 'w', 'a', 's', 'd'
    };


    for (int i = 0; i < KeyboardNumKeys; i++)
        keyboard_state.keys[i] = false;

    if (keys[VK_MENU   ] & 0x80) keyboard_state.alt   = true;
    if (keys[VK_CONTROL] & 0x80) keyboard_state.ctrl  = true;
    if (keys[VK_SHIFT  ] & 0x80) keyboard_state.shift = true;

    for (int i = 0; i < KeyboardNumKeys; i++) {
        if (!(keys[i] & 0x80))
            continue;

        for (int j = 0; j < KeyboardNumKeys; j++) {
            if (map_from[j] == i) {
                keyboard_state.keys[map_to[j]] = true;
                break;
            }
        }
    }
}

keyboardStateT getKeyboardState(void) {
    return (keyboard_state);
}

bool keyIsPressed(keyboardKeyT key) {
    return (keyboard_state.keys[key % KeyboardNumKeys]);
}

#endif // WIN32
