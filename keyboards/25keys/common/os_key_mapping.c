/* Copyright 2025 monksoffunk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// #include "quantum.h"
// #include "os_detection.h"

// bool set_os_key_mapping(os_variant_t os_type) {
//     switch (os_type) {
//         case OS_MACOS:
//         case OS_IOS:
//             keymap_config.swap_lalt_lgui = false;
//             break;
//         case OS_WINDOWS:
//         case OS_LINUX:
//             keymap_config.swap_lalt_lgui = true;
//             break;
//         default:
//             break;
//     }
//     return true;
// }
#include "quantum.h"
#include "os_detection.h"

bool set_os_key_mapping(os_variant_t os_type) {
    keymap_config.swap_lalt_lgui = false;
    return true;
}