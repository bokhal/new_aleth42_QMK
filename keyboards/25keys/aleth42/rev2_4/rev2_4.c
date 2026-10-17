#include "quantum.h"
#include "../common/matrix_remap.h"
#include "../common/os_key_mapping.h"

extern matrix_row_t matrix[MATRIX_ROWS]; // debounced values

void keyboard_post_init_kb(void) {
    wait_ms(500);
    // rgblight_set();
    keyboard_post_init_user();
}

bool process_detected_host_os_kb(os_variant_t os_type) {
    if (!process_detected_host_os_user(os_type)) {
        return false;
    }
    return set_os_key_mapping(os_type);
}

void matrix_scan_kb(void) {
    matrix_remap(matrix);
    matrix_scan_user();
}