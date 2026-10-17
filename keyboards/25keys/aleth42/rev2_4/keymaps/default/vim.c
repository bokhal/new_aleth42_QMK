#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "keymap.h"
#include "vim.h"

vim_state_t kb_vim = {0};

void reset_state_vim(void) {
    kb_vim.buffer_char = 0;
    kb_vim.count_vim = 0;
    kb_vim.stage = 0;
    kb_vim.move_stage = 0;
    kb_vim.executed = false;
}

static uint16_t vim_count(void) {
    return kb_vim.count_vim ? kb_vim.count_vim : 1;
}

static void tap16_vim(uint16_t keycode) {
    tap_code16(keycode);
    kb_vim.executed = true;
}
static void tap16_vim_count(uint16_t keycode) {
    for (uint16_t i = 0; i < vim_count(); i++) {
        tap_code16(keycode);
    }
    kb_vim.count_vim = 0;
    kb_vim.executed = true;
}

static void goto_anchor_line(void) {
    while (!(kb_vim.fromAnchorLine == 0)) {
        if (kb_vim.fromAnchorLine > 0) {
            tap16_vim(LSFT(KC_DOWN));
            kb_vim.fromAnchorLine--;
        } else {
            tap16_vim(LSFT(KC_UP));
            kb_vim.fromAnchorLine++;
        }
    }
}

static void goto_line(uint16_t line_number) {
    if (line_number == 0) { return; }

    const uint16_t MAX_JUMP = 700;
    if (line_number > MAX_JUMP) { return; }

    tap16_vim(LCTL(KC_HOME));
    for (uint16_t i = 1; i < line_number; i++) {
        tap16_vim(KC_DOWN);
    }
}

static bool keycode_to_char(uint16_t keycode, char *out) {
    if (keycode >= KC_A && keycode <= KC_Z) {
        *out = 'a' + (keycode - KC_A);
        return true;
    }
    if (keycode >= KC_1 && keycode <= KC_9) {
        *out = '1' + (keycode - KC_1);
        return true;
    }
    if (keycode == KC_0) {
        *out = '0';
        return true;
    }
    return false;
}

// up == true  → K(위로 확장), up == false → J(아래로 확장)
static void visual_line_move(bool up) {
    const uint16_t move_key = up ? KC_UP : KC_DOWN;
    const uint16_t near_key = up ? KC_END : KC_HOME;  // 앵커라인 기준 "같은 쪽" 끝
    const uint16_t far_key  = up ? KC_HOME : KC_END;  // 반대쪽 끝

    kb_vim.fromAnchorLine += up ? 1 : -1;
    const int16_t crossing_value = up ? 1 : -1;

    if (kb_vim.fromAnchorLine == crossing_value) {
        // 앵커라인을 막 넘어가는 순간: 커서를 반대쪽 끝으로 스냅
        tap16_vim(near_key);
        tap16_vim(LSFT(move_key));
        tap16_vim(LSFT(far_key));
        return;
    }

    bool same_side = up ? (kb_vim.fromAnchorLine <= 0) : (kb_vim.fromAnchorLine >= 0);
    tap16_vim(LSFT(move_key));
    tap16_vim(LSFT(same_side ? near_key : far_key));
}

static void vim_move_process(uint16_t keycode) {
    switch(kb_vim.move_stage) {
        case 0:
            switch(keycode) {
                case KC_K: // 상
                    if (kb_vim.visual_shifted) {
                        visual_line_move(true);
                        break;
                    }

                    tap16_vim(KC_UP);
                    break;

                case KC_J: // 하
                    if (kb_vim.visual_shifted) {
                        visual_line_move(false);
                        break;
                    }

                    tap16_vim(KC_DOWN);
                    break;

                case KC_H: // 좌
                    if (kb_vim.visual_shifted) { break; }

                    tap16_vim(KC_LEFT);
                    break;

                case KC_L: // 우
                    if (kb_vim.visual_shifted) { break; }

                    tap16_vim(KC_RGHT);
                    break;

                case KC_MINS: // 줄의 처음위치
                    if (kb_vim.visual_shifted) { break; }

                    tap16_vim(KC_HOME);
                    break;

                case KC_G: // 맨 아래로 이동
                    if (is_shifted()) {
                        if (kb_vim.visual_shifted) {
                            goto_anchor_line();
                            tap16_vim(KC_HOME);
                            tap16_vim(LCTL(LSFT(KC_END)));
                            break;
                        }

                        tap16_vim(LCTL(KC_END));
                    }
                    break;

                case KC_W: // 단어의 시작위치 (forward)
                    if (kb_vim.visual_shifted) { break; }

                    tap16_vim_count(LCTL(KC_RGHT));
                    tap16_vim(LCTL(KC_LEFT));
                    break;

                case KC_E: // 단어의 마지막위치 (forward)
                    if (kb_vim.visual_shifted) { break; }

                    tap16_vim_count(LCTL(KC_RGHT));
                    break;

                case KC_B: // 단어의 시작위치 (backward)
                    if (kb_vim.visual_shifted) { break; }

                    tap16_vim_count(LCTL(KC_LEFT));
                    break;
            }
            break;

        case 1:
            switch(keycode) {
                case KC_G: // 맨 위 이동
                    if (kb_vim.buffer_char == KC_G) {
                        if(kb_vim.visual_shifted) {
                            goto_anchor_line();
                            tap16_vim(KC_END);
                            tap16_vim(LCTL(LSFT(KC_HOME)));
                            break;
                        }

                        tap16_vim(LCTL(KC_HOME));
                    }
                    kb_vim.executed = true;
                    break;

                case KC_E: // 단어의 마지막위치 (backward)
                    if (kb_vim.visual_shifted) { break; }

                    if (kb_vim.buffer_char == KC_G) {
                        tap16_vim_count(LCTL(KC_LEFT));
                        tap16_vim(LCTL(KC_RGHT));
                    }
                    kb_vim.executed = true;
                    break;

            }
            break;
    }
}

bool mode_vim_normal_process(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return false;

    if (keycode == KC_ESC) {
        reset_state_vim();
        return false;
    }

    if (keycode >= KC_1 && keycode <= KC_9) {
        kb_vim.count_vim = kb_vim.count_vim * 10 + (keycode - KC_1 + 1);
        return false;
    } else if (keycode == KC_0 && kb_vim.count_vim != 0) {
        kb_vim.count_vim *= 10;
        return false;
    }

    vim_move_process(keycode);
    switch(kb_vim.stage) {

        case 0:
            switch(keycode){
                // 모드 전환
                case KC_V: // visual 모드 진입
                    kb_mode = MODE_VIM_VISUAL;
                    kb_vim.visual_shifted = is_shifted();
                    if (kb_vim.visual_shifted) {
                        tap16_vim(KC_END);
                        tap16_vim(LSFT(KC_HOME));
                    }

                    kb_vim.executed = true;
                    break;

                case KC_SCLN:
                    if (!is_shifted()) { break; }

                    kb_mode = MODE_VIM_COMMAND;
                    kb_vim.cmd_len = 0;
                    kb_vim.cmd_buffer[0] = '\0';
                    kb_vim.executed = true;
                    break;

                // 편집
                case KC_I:
                    if (is_shifted()) { // 현재 커서 줄 맨 앞 insert
                        tap16_vim(KC_HOME);
                        kb_mode = MODE_VIM_INSERT;
                    } else {            // 현재 커서 앞 insert
                        tap16_vim(KC_LEFT);
                        kb_mode = MODE_VIM_INSERT;
                    }
                    break;

                case KC_A:
                    if (is_shifted()) { // 현재 커서 줄 맨 뒤 insert
                        tap16_vim(KC_END);
                        kb_mode = MODE_VIM_INSERT;
                    } else {            // 현재 커서 뒤 insert
                        tap16_vim(KC_RGHT);
                        kb_mode = MODE_VIM_INSERT;
                    }
                    break;

                case KC_O:
                    if (is_shifted()) { // 현재 커서 윗줄 insert
                        tap16_vim(KC_HOME);
                        tap16_vim(KC_ENT);
                        tap16_vim(KC_UP);
                        kb_mode = MODE_VIM_INSERT;
                    } else {            // 현재 커서 아랫줄 insert
                        tap16_vim(KC_END);
                        tap16_vim(KC_ENT);
                        kb_mode = MODE_VIM_INSERT;
                    }
                    break;

                case KC_S: // 현재 커서 줄 잘라내고 insert
                    if (is_shifted()) {
                        tap16_vim(KC_HOME);
                        tap16_vim(LSFT(KC_END));
                        tap16_vim(LCTL(KC_X));
                        kb_mode = MODE_VIM_INSERT;
                    } else {
                        kb_vim.buffer_char = keycode;
                        kb_vim.stage++;
                    }
                    break;

                case KC_X: // 현재 커서 문자 잘라내기
                    tap16_vim(LSFT(KC_RGHT));
                    tap16_vim(LCTL(KC_X));
                    break;

                case KC_P:
                    if (is_shifted()) { // 현재 커서 아랫줄 붙여넣기
                        tap16_vim(KC_END);
                        tap16_vim(KC_ENT);
                        tap16_vim(LCTL(KC_V));
                    } else { // 현재 커서 뒤 붙여넣기
                        tap16_vim(LCTL(KC_V));
                    }
                    break;

                case KC_U: // Undo
                    tap16_vim(LCTL(KC_Z));
                    break;

                case KC_R: // Redo
                    if (is_ctrled()) {
                        tap16_vim(LCTL(LSFT(KC_Z)));
                    }
                    break;

                default:
                    kb_vim.buffer_char = keycode;
                    kb_vim.stage++;
                    break;
            }
            break;

        case 1:
            switch (keycode) {
                // 편집
                case KC_D: // 현재 커서 줄 잘라내기
                    if (kb_vim.buffer_char == KC_D) {
                        tap16_vim(KC_HOME);
                        tap16_vim(LSFT(KC_END));
                        tap16_vim(LCTL(KC_X));
                    }
                    kb_vim.executed = true;
                    break;

                case KC_C: // 현재 커서 줄 잘라내고 insert
                    if (kb_vim.buffer_char == KC_C) {
                        tap16_vim(KC_HOME);
                        tap16_vim(LSFT(KC_END));
                        tap16_vim(LCTL(KC_X));
                        kb_mode = MODE_VIM_INSERT;
                    }
                    kb_vim.executed = true;
                    break;

                case KC_W:
                    switch (kb_vim.buffer_char) {
                        case KC_C: // 현재 커서 위치부터 단어의 마지막까지 잘라내고 insert
                            tap16_vim(LSFT(KC_END));
                            tap16_vim(LCTL(KC_X));
                            kb_mode = MODE_VIM_INSERT;
                            break;

                        case KC_Y: // 현재 커서 위치부터 단어 끝까지 복사하기
                            tap16_vim(LSFT(KC_END));
                            tap16_vim(LCTL(KC_C));
                            break;
                    }
                    kb_vim.executed = true;
                    break;

                case KC_L: // 현재 커서 위치 문자 하나 복사하기
                    if (kb_vim.buffer_char == KC_Y) {
                        tap16_vim_count(LSFT(KC_RGHT));
                        tap16_vim(LCTL(KC_C));
                    }
                    kb_vim.executed = true;
                    break;

                case KC_Y: // 현재 커서 위치 줄 복사하기
                    if (kb_vim.buffer_char == KC_Y) {
                        tap16_vim(KC_HOME);
                        tap16_vim(LSFT(KC_END));
                        tap16_vim(LCTL(KC_C));
                    }
                    kb_vim.executed = true;
                    break;

                default:
                    kb_vim.executed = true;
                    break;
            }
            break;
    }

    if (kb_vim.executed) {
        reset_state_vim();
    }

    return false;
}

bool mode_vim_insert_process(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_ESC && record->event.pressed) {
        kb_mode = MODE_VIM_NORMAL;
        return false;
    }

    return typing_process(keycode, record);
}

bool mode_vim_visual_process(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return false;

    if (keycode == KC_ESC) {
        kb_mode = MODE_VIM_NORMAL;
        kb_vim.fromAnchorLine = 0;
        kb_vim.visual_shifted = false;
        return false;
    }

    switch (keycode) {
        case KC_D: // 선택영역 삭제
            tap16_vim(KC_DEL);
            break;

        case KC_Y: // 선택영역 복사
            tap16_vim(LCTL(KC_C));
            break;

        case KC_C: // 선택영역 삭제 후 insert 진입
            tap16_vim(KC_DEL);
            kb_mode = MODE_VIM_INSERT;
            reset_state_vim();
            kb_vim.fromAnchorLine = 0;
            kb_vim.visual_shifted = false;
            break;

        case KC_O: // anchor line을 cursor line으로 설정
            kb_vim.fromAnchorLine = 0;
            break;

        default:
            vim_move_process(keycode);
            break;
    }

    if (kb_vim.executed) {
        reset_state_vim();
    }

    return false;
}

void reset_vim_command_buffer(void) {
    kb_vim.cmd_len = 0;
    kb_vim.cmd_buffer[0] = '\0';
}

static void vim_command_cancel(void) {
    kb_mode = MODE_VIM_NORMAL;
    reset_vim_command_buffer();
}

static void execute_vim_command(const char *cmd) {
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "gg") == 0) {
        kb_mode = MODE_BASE;
        reset_state_vim();
        kb_vim.fromAnchorLine = 0;
        kb_vim.visual_shifted = false;
    } else if (strcmp(cmd, "cw") == 0) {
        tap16_vim(QK_CAPS_WORD_TOGGLE);
    } else if (strcmp(cmd, "help") == 0) {
        SEND_STRING("my vim url");
    } else if (isdigit((unsigned char)cmd[0])) {
        goto_line(atoi(cmd));
    }
}

bool mode_vim_command_process(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) { return false; }

    if (keycode == KC_ESC) {
        vim_command_cancel();
        return false;
    }

    if (keycode == KC_ENT) {
        kb_vim.cmd_buffer[kb_vim.cmd_len] = '\0';
        execute_vim_command(kb_vim.cmd_buffer);

        reset_vim_command_buffer();
        return false;
    }

    if (keycode == KC_BSPC) {
        if (kb_vim.cmd_len > 0) {
            kb_vim.cmd_len--;
            kb_vim.cmd_buffer[kb_vim.cmd_len] = '\0';
        }
        return false;
    }

    char c;
    if (keycode_to_char(keycode, &c)) {
        if (kb_vim.cmd_len < VIM_CMD_BUF_SIZE - 1) {
            kb_vim.cmd_buffer[kb_vim.cmd_len++] = c;
        }
    }

    return false;
}