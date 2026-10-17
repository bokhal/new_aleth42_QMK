#pragma once

#include QMK_KEYBOARD_H

enum custom_keycodes {
    MACRO = SAFE_RANGE,
    TAB_ARROW
};

bool is_shifted(void);
bool is_ctrled(void);
bool typing_process(uint16_t keycode, keyrecord_t *record);