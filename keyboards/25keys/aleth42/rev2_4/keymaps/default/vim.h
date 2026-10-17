#pragma once

#include QMK_KEYBOARD_H

#define VIM_CMD_BUF_SIZE 16

typedef enum {
    MODE_BASE,
    MODE_VIM_NORMAL,
    MODE_VIM_INSERT,
    MODE_VIM_VISUAL,
    MODE_VIM_COMMAND
} keyboard_mode_t;

typedef struct {
    uint16_t count_vim;
    uint16_t buffer_char;
    uint8_t  stage;
    uint8_t  move_stage;
    int16_t  fromAnchorLine;
    bool     executed;
    bool     visual_shifted;

    char     cmd_buffer[VIM_CMD_BUF_SIZE];
    uint8_t  cmd_len;
} vim_state_t;

extern keyboard_mode_t kb_mode;
extern vim_state_t kb_vim;

bool mode_vim_normal_process(uint16_t keycode, keyrecord_t *record);
bool mode_vim_insert_process(uint16_t keycode, keyrecord_t *record);
bool mode_vim_visual_process(uint16_t keycode, keyrecord_t *record);
bool mode_vim_command_process(uint16_t keycode, keyrecord_t *record);

void reset_state_vim(void);
void reset_vim_command_buffer(void);