#include QMK_KEYBOARD_H



// Добавлено на пробу
#define TAPPING_TERM 175
#define TAPPING_TERM_PER_KEY

// Tap Dance declarations
enum {
    TD_ESC_CAPS,
};

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Escape, twice for Caps Lock
    [TD_ESC_CAPS] = ACTION_TAP_DANCE_DOUBLE(KC_ESC, KC_CAPS),
};

// Add tap dance item to your keymap in place of a keycode
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // ...
    TD(TD_ESC_CAPS)
    // ...
};

void dance_egg(tap_dance_state_t *state, void *user_data) {
    if (state->count >= 100) {
        SEND_STRING("Safety dance!");
        reset_tap_dance(state);
    }
}

tap_dance_action_t tap_dance_actions[] = {
    [CT_EGG] = ACTION_TAP_DANCE_FN(dance_egg),
};

// On each tap, light up one LED, from right to left
// On the fourth tap, turn them off from right to left
void dance_flsh_each(tap_dance_state_t *state, void *user_data) {
    switch (state->count) {
        case 1:
            ergodox_right_led_3_on();
            break;
        case 2:
            ergodox_right_led_2_on();
            break;
        case 3:
            ergodox_right_led_1_on();
            break;
        case 4:
            ergodox_right_led_3_off();
            wait_ms(50);
            ergodox_right_led_2_off();
            wait_ms(50);
            ergodox_right_led_1_off();
    }
}

// On the fourth tap, set the keyboard on flash state
void dance_flsh_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count >= 4) {
        reset_keyboard();
    }
}

// If the flash state didn't happen, then turn off LEDs, left to right
void dance_flsh_reset(tap_dance_state_t *state, void *user_data) {
    ergodox_right_led_1_off();
    wait_ms(50);
    ergodox_right_led_2_off();
    wait_ms(50);
    ergodox_right_led_3_off();
}

// All tap dances now put together. Example 2 is "CT_FLSH"
tap_dance_action_t tap_dance_actions[] = {
    [TD_ESC_CAPS] = ACTION_TAP_DANCE_DOUBLE(KC_ESC, KC_CAPS),
    [CT_EGG] = ACTION_TAP_DANCE_FN(dance_egg),
    [CT_FLSH] = ACTION_TAP_DANCE_FN_ADVANCED(dance_flsh_each, dance_flsh_finished, dance_flsh_reset)
};



// Пример 3. Отправка `:` при касании, `;` - при удержании
typedef struct {
    uint16_t tap;
    uint16_t hold;
    uint16_t held;
} tap_dance_tap_hold_t;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    tap_dance_action_t *action;

    switch (keycode) {
        case TD(CT_CLN):  // list all tap dance keycodes with tap-hold configurations
            action = &tap_dance_actions[TD_INDEX(keycode)];
            if (!record->event.pressed && action->state.count && !action->state.finished) {
                tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)action->user_data;
                tap_code16(tap_hold->tap);
            }
    }
    return true;
}

void tap_dance_tap_hold_finished(tap_dance_state_t *state, void *user_data) {
    tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)user_data;

    if (state->pressed) {
        if (state->count == 1
#ifndef PERMISSIVE_HOLD
            && !state->interrupted
#endif
        ) {
            register_code16(tap_hold->hold);
            tap_hold->held = tap_hold->hold;
        } else {
            register_code16(tap_hold->tap);
            tap_hold->held = tap_hold->tap;
        }
    }
}

void tap_dance_tap_hold_reset(tap_dance_state_t *state, void *user_data) {
    tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)user_data;

    if (tap_hold->held) {
        unregister_code16(tap_hold->held);
        tap_hold->held = 0;
    }
}

#define ACTION_TAP_DANCE_TAP_HOLD(tap, hold) \
    { .fn = {NULL, tap_dance_tap_hold_finished, tap_dance_tap_hold_reset}, .user_data = (void *)&((tap_dance_tap_hold_t){tap, hold, 0}), }

tap_dance_action_t tap_dance_actions[] = {
    [CT_CLN] = ACTION_TAP_DANCE_TAP_HOLD(KC_COLN, KC_SCLN),
};



// Пример 4: «Стечетка четырех функций»

/**
 * 
 * Разрешить одной клавише иметь 4 (или более) функции, в зависимости от количества нажатий, а также от того, удерживается ли клавиша или нажимается. Ниже приведен конкретный пример:

  Нажмите = Отправитьx
  Удерживать = ОтправитьControl
  Двойное нажатие = отправитьEscape
  Двойное нажатие и удержание = отправитьAlt
  Вам понадобится несколько вещей, которые можно использовать для «чечетки с четырьмя функциями».

  Вам нужно будет добавить их в начало файла keymap.cперед раскладкой.  
*/
typedef enum {
    TD_NONE,
    TD_UNKNOWN,
    TD_SINGLE_TAP,
    TD_SINGLE_HOLD,
    TD_DOUBLE_TAP,
    TD_DOUBLE_HOLD,
    TD_DOUBLE_SINGLE_TAP, // Send two single taps
    TD_TRIPLE_TAP,
    TD_TRIPLE_HOLD
} td_state_t;

typedef struct {
    bool is_press_action;
    td_state_t state;
} td_tap_t;

// Tap dance enums
enum {
    X_CTL,
    SOME_OTHER_DANCE
};

td_state_t cur_dance(tap_dance_state_t *state);

// For the x tap dance. Put it here so it can be used in any keymap
void x_finished(tap_dance_state_t *state, void *user_data);
void x_reset(tap_dance_state_t *state, void *user_data);
