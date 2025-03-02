/* Copyright 2022 @ Keychron (https://www.keychron.com)
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

#include "keychron_common.h"
#include "mousekey.h"
#include "report.h"
#include "sync_timer.h"

const int JIG_STEP_SIZE  = 10;
bool      is_siri_active = false;
uint32_t  siri_timer     = 0;

bool     is_mouse_jigler_active = false;
uint32_t mouse_jigler_timer     = 0;
uint8_t  state                  = 0;

key_combination_t key_comb_list[4] = {{2, {KC_LWIN, KC_TAB}}, {2, {KC_LWIN, KC_E}}, {3, {KC_LSFT, KC_LCMD, KC_4}}, {2, {KC_LWIN, KC_C}}};

static uint8_t mac_keycode[4] = {KC_LOPT, KC_ROPT, KC_LCMD, KC_RCMD};

report_mouse_t mousekey_task_keychron(report_mouse_t mouse_report) {
    if (is_mouse_jigler_active && sync_timer_elapsed32(mouse_jigler_timer) >= 500) {
        mouse_jigler_timer = sync_timer_read32();

        switch (state) {
            case 0:
                mouse_report.x = JIG_STEP_SIZE;
                mouse_report.y = 0;
                state++;
                break;
            case 1:
                mouse_report.x = 0;
                mouse_report.y = JIG_STEP_SIZE;
                state++;
                break;
            case 2:
                mouse_report.x = -JIG_STEP_SIZE;
                mouse_report.y = 0;
                state++;
                break;
            case 3:
                mouse_report.x = 0;
                mouse_report.y = -JIG_STEP_SIZE;
                state++;
                break;
            default:
                mouse_report.x = 0;
                mouse_report.y = 0;
                state          = 0;
        }
    }
    return mouse_report;
}

void housekeeping_task_keychron(void) {
    if (is_siri_active) {
        if (sync_timer_elapsed32(siri_timer) >= 500) {
            unregister_code(KC_LCMD);
            unregister_code(KC_SPACE);
            is_siri_active = false;
        }
    }
}

bool process_record_keychron(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case QK_KB_0:
            if (record->event.pressed) {
                register_code(KC_MISSION_CONTROL);
            } else {
                unregister_code(KC_MISSION_CONTROL);
            }
            return false; // Skip all further processing of this key
        case QK_KB_1:
            if (record->event.pressed) {
                register_code(KC_LAUNCHPAD);
            } else {
                unregister_code(KC_LAUNCHPAD);
            }
            return false; // Skip all further processing of this key
        case KC_LOPTN:
        case KC_ROPTN:
        case KC_LCMMD:
        case KC_RCMMD:
            if (record->event.pressed) {
                register_code(mac_keycode[keycode - KC_LOPTN]);
            } else {
                unregister_code(mac_keycode[keycode - KC_LOPTN]);
            }
            return false; // Skip all further processing of this key
        case KC_SIRI:
            if (record->event.pressed) {
                if (!is_siri_active) {
                    is_siri_active = true;
                    register_code(KC_LCMD);
                    register_code(KC_SPACE);
                }
                siri_timer = sync_timer_read32();
            } else {
                // Do something else when release
            }
            return false; // Skip all further processing of this key
        case KC_TASK:
        case KC_FLXP:
        case KC_SNAP:
        case KC_CRTA:
            if (record->event.pressed) {
                for (uint8_t i = 0; i < key_comb_list[keycode - KC_TASK].len; i++) {
                    register_code(key_comb_list[keycode - KC_TASK].keycode[i]);
                }
            } else {
                for (uint8_t i = 0; i < key_comb_list[keycode - KC_TASK].len; i++) {
                    unregister_code(key_comb_list[keycode - KC_TASK].keycode[i]);
                }
            }
            return false; // Skip all further processing of this key
        case KC_TG_JIGLER:
            if (record->event.pressed) {
                mouse_jigler_timer     = sync_timer_read32();
                is_mouse_jigler_active = !is_mouse_jigler_active;
            }
            return false;
        default:
            return true; // Process all other keycodes normally
    }
}
