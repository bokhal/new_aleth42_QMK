#include QMK_KEYBOARD_H
#include "../../common/40_keymap.h"
#include "os_detection.h"
#include "keymap.h"

// === === === OS helpers === === ===
static os_variant_t get_os(void) {
    return detected_host_os();
}

static bool is_os_windows(void) {
    return get_os() == OS_WINDOWS;
}

// === === === Declaration === === ===
typedef struct {
    bool pressed;
    bool hold;
    uint16_t timer;
    uint16_t tap_key;
    uint16_t hold_delay;
} hold_key_t;

static bool macro_pressed = false;

static hold_key_t tab = {
    .pressed = false,
    .hold = false,
    .timer = 0,
    .tap_key = KC_TAB,
    .hold_delay = 200
};

// --- 방향키 레이어 반복 입력 ---
#define ARROW_REPEAT_DELAY    300  // 처음 반복이 시작되기까지 (ms)
#define ARROW_REPEAT_INTERVAL 30   // 반복 간격 (ms)

static uint16_t repeat_key = KC_NO;     // 눌려 있는 물리 키 (KC_NO면 반복 없음)
static uint16_t repeat_code = KC_NO;    // 반복해서 보낼 키코드
static uint16_t repeat_timer = 0;
static bool     repeat_started = false; // 첫 딜레이 이후 true

static void stop_repeat(void) {
    repeat_key = KC_NO;
    repeat_code = KC_NO;
    repeat_started = false;
}

// === === === enums === === ===
enum layer_names {
    _BASE,
    _FN,
    _SYS
};

enum combos {
    CW_CAPSWORD,
    NM_EMAIL,
    PS_SCREENSHOT
};

// === === === Combos === === ===
const uint16_t PROGMEM capsWord_combo[] = {KC_C, KC_W, COMBO_END};
const uint16_t PROGMEM email_combo[] = {KC_N, KC_M, COMBO_END};
const uint16_t PROGMEM screenShot_combo[] = {KC_P, KC_S, COMBO_END};

combo_t key_combos[] = {
    [CW_CAPSWORD] = COMBO(capsWord_combo, QK_CAPS_WORD_TOGGLE),
    [NM_EMAIL] = COMBO_ACTION(email_combo),
    [PS_SCREENSHOT] = COMBO_ACTION(screenShot_combo)
};

static void send_screenshot_shortcut(void) {
    switch (get_os()) {
        case OS_WINDOWS:
            tap_code16(LGUI(LSFT(KC_S)));
            break;
        case OS_MACOS:
            tap_code16(LCMD(LSFT(KC_4)));
            break;
        case OS_LINUX:
            tap_code16(LCTL(LSFT(KC_PSCR)));
            break;
        default:
            break;
    }
}

void process_combo_event(uint16_t combo_index, bool pressed) {
    if (!pressed) return;

    switch (combo_index) {
        case NM_EMAIL:
            SEND_STRING("bokhal451@gmail.com");
            break;

        case PS_SCREENSHOT:
            send_screenshot_shortcut();
            break;
    }
}

// === === === Keymaps === === ===
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_wrapper(
        KC_ESC,         KC_Q,  KC_W,  KC_E,  KC_R,  KC_T,  KC_Y,  KC_U,  KC_I,  KC_O,  KC_P,   KC_MUTE,
        TAB_ARROW,     KC_A,  KC_S,  KC_D,  KC_F,  KC_G,  KC_H,  KC_J,  KC_K,  KC_L,       KC_ENT,
        KC_LSFT,             KC_Z,  KC_X,  KC_C,  KC_V,  KC_B,  KC_N,  KC_M,  KC_COMM, KC_DOT, KC_SLSH,
        KC_LCTL,     KC_LGUI, KC_LALT,          LT(_FN, KC_BSPC),           KC_SPC,        MO(_SYS),  KC_RSFT, KC_MPLY
    ),

    [_FN] = LAYOUT_wrapper(
        MACRO, KC_1, KC_2, KC_3, KC_4, KC_5, KC_6, KC_7, KC_8, KC_9, KC_0, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_DEL, KC_HOME, KC_PGUP, KC_QUOT, KC_LBRC, KC_RBRC, KC_SCLN, KC_ENT,
        KC_LSFT, KC_NO, KC_NO, KC_END, KC_PGDN, KC_NO, KC_MINS, KC_EQL, KC_NO, KC_UP, KC_BSLS,
        KC_NO, KC_NO, KC_NO, KC_TRNS, KC_BSPC, KC_LEFT, KC_DOWN, KC_RGHT
    ),

    [_SYS] = LAYOUT_wrapper(
        KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11, KC_F12,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS, KC_NO, KC_NO
    )
};

// === === === Encoder === === ===
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [_BASE] = {
        ENCODER_CCW_CW(KC_NO, KC_NO),
        ENCODER_CCW_CW(KC_VOLD, KC_VOLU),
    },

    [_FN] = {
        ENCODER_CCW_CW(KC_NO, KC_NO),
        ENCODER_CCW_CW(MS_WHLL, MS_WHLR),
    },

    [_SYS] = {
        ENCODER_CCW_CW(KC_NO, KC_NO),
        ENCODER_CCW_CW(LCTL(LSFT(KC_TAB)), LCTL(KC_TAB)),
    }
};
#endif

// === === === Key override === === ===
const key_override_t tilde_override =
    ko_make_basic(MOD_MASK_SHIFT, KC_ESC, KC_TILD);

const key_override_t *key_overrides[] = {
    &tilde_override,
    NULL
};

// === === === hold key === === ===
static void process_hold_key(hold_key_t *key, keyrecord_t *record) {
    if (record->event.pressed) {
        key->pressed = true;
        key->timer = timer_read();
    } else {
        if (timer_elapsed(key->timer) < key->hold_delay) {
            tap_code(key->tap_key);
        }
        key->pressed = false;
        key->hold = false;
        key->timer = 0;
        stop_repeat();
    }
}

// === === === macro / arrow layer === === ===
static bool macro_layer(uint16_t k, keyrecord_t *r) {
    if (!r->event.pressed) return true;

    switch (k) {
        case KC_1:
            SEND_STRING("andy52599731@gmail.com");
            return false;

        case KC_2:
            SEND_STRING("bokhal451@gmail.com");
            return false;

        default:
            return true;
    }
}

static bool arrow_layer(uint16_t k, keyrecord_t *r) {
    if (!r->event.pressed) {
        if (k == repeat_key) {
            stop_repeat();
            return false;
        }
        return true;
    }

    uint16_t code;
    bool repeatable = true;

    switch (k) {
        case KC_Q:
            code = LCTL(KC_TAB);
            repeatable = false;  // 탭 전환은 반복하지 않음
            break;

        case KC_W:
            code = KC_UP;
            break;

        case KC_E:
            code = LCTL(S(KC_TAB));
            repeatable = false;
            break;

        case KC_A:
            code = KC_LEFT;
            break;

        case KC_S:
            code = KC_DOWN;
            break;

        case KC_D:
            code = KC_RGHT;
            break;

        default:
            return true;
    }

    tap_code16(code);

    if (repeatable) {
        repeat_key = k;
        repeat_code = code;
        repeat_timer = timer_read();
        repeat_started = false;
    }
    return false;
}

// === === === typing / mode dispatch === === ===
static bool typing_process(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case MACRO:
            macro_pressed = record->event.pressed;
            return false;

        case TAB_ARROW:
            process_hold_key(&tab, record);
            return false;

        case KC_SPC:
            if (record->event.pressed && (get_mods() & MOD_MASK_CTRL)) {
                if (is_os_windows()) {
                    tap_code(KC_LNG2);
                    return false;
                }
            }
            break;
    }

    if (macro_pressed) {
        return macro_layer(keycode, record);
    }

    if (tab.pressed && !tab.hold && keycode != TAB_ARROW && record->event.pressed) {
        tab.hold = true;
    }

    if (tab.hold) {
        return arrow_layer(keycode, record);
    }

    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return typing_process(keycode, record);
}

// === === === matrix scan === === ===
void matrix_scan_user(void) {
    if (tab.pressed && !tab.hold) {
        if (timer_elapsed(tab.timer) > tab.hold_delay) {
            tab.hold = true;
        }
    }

    if (repeat_key != KC_NO) {
        uint16_t wait = repeat_started ? ARROW_REPEAT_INTERVAL : ARROW_REPEAT_DELAY;
        if (timer_elapsed(repeat_timer) >= wait) {
            tap_code16(repeat_code);
            repeat_timer = timer_read();
            repeat_started = true;
        }
    }
}