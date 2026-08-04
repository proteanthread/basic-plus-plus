/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#include "vm/vm.h"
#include "vm_internal.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "types/config.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BPP_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "device/usb.h"
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/bus.h"
#include "bios/mock_bios.h"
#include "runtime/spec.h"
#include "security/security.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "runtime/variables.h"
#include "platform/platform.h"
#include "core/dialect.h"
#include "core/struct.h"
#include "module/module.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


double platform_get_timer(void);
double platform_get_uptime(void);
int platform_inkey_char(void);
#ifndef BPP_LITE_BUILD
int vdev_music_note_count(void);
#endif

void vm_set_timer_trap(VMContext *vm, double seconds, BppLineNumber line) {
    if (!vm) return;
    vm->timer_interval = seconds;
    vm->timer_gosub_line = line;
    vm->timer_last_trigger = platform_get_uptime();
}

void vm_set_timer_state(VMContext *vm, int state) {
    if (!vm) return;
    vm->timer_state = state;
    if (state == 1) {
        if (vm->timer_pending) {
            vm->timer_pending = false;
        }
    }
}

void vm_set_key_trap(VMContext *vm, int key_idx, int key_code, BppLineNumber line) {
    if (!vm || key_idx < 1 || key_idx >= 15100) return;
    vm->key_code[key_idx] = key_code;
    vm->key_gosub_line[key_idx] = line;
}

void vm_set_key_state(VMContext *vm, int key_idx, int state) {
    if (!vm || key_idx < 1 || key_idx >= 15100) return;
    vm->key_state[key_idx] = state;
}

int vm_get_key_state(const VMContext *vm, int key_idx) {
    if (!vm || key_idx < 1 || key_idx >= 15100) return 0;
    return vm->key_state[key_idx];
}

int vm_get_key_code(const VMContext *vm, int key_idx) {
    if (!vm || key_idx < 1 || key_idx >= 15100) return 0;
    return vm->key_code[key_idx];
}

BppLineNumber vm_get_key_gosub_line(const VMContext *vm, int key_idx) {
    if (!vm || key_idx < 1 || key_idx >= 15100) return 0.0;
    return vm->key_gosub_line[key_idx];
}

void vm_set_usr_ptr(VMContext *vm, int idx, uintptr_t ptr) {
    if (!vm || idx < 0 || idx > 9) return;
    vm->usr_pointers[idx] = ptr;
}

uintptr_t vm_get_usr_ptr(const VMContext *vm, int idx) {
    if (!vm || idx < 0 || idx > 9) return 0;
    return vm->usr_pointers[idx];
}

void vm_set_com_trap(VMContext *vm, int port_idx, BppLineNumber line) {
    if (!vm || port_idx < 1 || port_idx > 4) return;
    vm->com_gosub_line[port_idx] = line;
}

void vm_set_com_state(VMContext *vm, int port_idx, int state) {
    if (!vm || port_idx < 1 || port_idx > 4) return;
    vm->com_state[port_idx] = state;
}

void vm_set_pen_trap(VMContext *vm, BppLineNumber line) {
    if (!vm) return;
    vm->pen_gosub_line = line;
}

void vm_set_pen_state(VMContext *vm, int state) {
    if (!vm) return;
    vm->pen_state = state;
}

void vm_set_strig_trap(VMContext *vm, int strig_idx, BppLineNumber line) {
    if (!vm || strig_idx < 0 || strig_idx > 4) return;
    vm->strig_gosub_line[strig_idx] = line;
}

void vm_set_strig_state(VMContext *vm, int strig_idx, int state) {
    if (!vm || strig_idx < 0 || strig_idx > 4) return;
    vm->strig_state[strig_idx] = state;
}

void vm_set_play_trap(VMContext *vm, int note_threshold, BppLineNumber line) {
    if (!vm) return;
    vm->play_note_threshold = note_threshold;
    vm->play_gosub_line = line;
}

void vm_set_play_state(VMContext *vm, int state) {
    if (!vm) return;
    vm->play_state = state;
}

void vm_set_mouse_trap(VMContext *vm, int state, BppLineNumber line, int target_char) {
    if (!vm) return;
    if (state != -1) vm->mouse_state = state;
    if (line != -1.0) vm->mouse_gosub_line = line;
    if (target_char != -2) vm->mouse_target_char = target_char; /* use -2 as sentinel for not changing */
}

void vm_set_hmouse_trap(VMContext *vm, int state, BppLineNumber line) {
    if (!vm) return;
    if (state != -1) vm->hmouse_state = state;
    if (line != -1.0) vm->hmouse_gosub_line = line;
}

void vm_set_vmouse_trap(VMContext *vm, int state, BppLineNumber line) {
    if (!vm) return;
    if (state != -1) vm->vmouse_state = state;
    if (line != -1.0) vm->vmouse_gosub_line = line;
}

void vm_set_trig_trap(VMContext *vm, int state, BppLineNumber line, int target_btn) {
    if (!vm) return;
    if (state != -1) vm->trig_state = state;
    if (line != -1.0) vm->trig_gosub_line = line;
    if (target_btn != -2) vm->trig_target_btn = target_btn; /* use -2 as sentinel for not changing */
}

void vm_trigger_event_polling(VMContext *vm) {
    if (!vm || !vm->running) return;

    /* 1. Timer Check */
    if (vm->timer_state == 1 && vm->timer_interval > 0.0) {
        double now = platform_get_uptime();
        if (now - vm->timer_last_trigger >= vm->timer_interval) {
            if (!vm->in_timer_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                vm->in_timer_handler = true;
                vm->timer_last_trigger = now;
                if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                    vm_jump(vm, vm->timer_gosub_line, NULL);
                    return;
                }
            } else if (vm->in_timer_handler) {
                vm->timer_last_trigger = now;
            }
        }
    } else if (vm->timer_state == 2 && vm->timer_interval > 0.0) {
        double now = platform_get_uptime();
        if (now - vm->timer_last_trigger >= vm->timer_interval) {
            vm->timer_pending = true;
            vm->timer_last_trigger = now;
        }
    }

    /* 2. Key Check */
    int key_pressed = platform_inkey_char();
    if (key_pressed > 0) {
        int scan = key_pressed & 0xFF;
        int mods = (key_pressed >> 8) & 0x1FF;
        int F_key = 0;
        if (scan >= 59 && scan <= 68) {
            F_key = scan - 58;
        } else if (scan == 133) {
            F_key = 11;
        } else if (scan == 134) {
            F_key = 12;
        }

        int idx = -1;
        if (F_key > 0) {
            int P_Shift = 0;
            bool s_l = (mods & 1);
            bool s_r = (mods & 16);
            if (s_l && s_r) P_Shift = 4;
            else if (s_l) P_Shift = 1;
            else if (s_r) P_Shift = 2;

            int P_Ctrl = 0;
            bool c_l = (mods & 2);
            bool c_r = (mods & 32);
            if (c_l && c_r) P_Ctrl = 4;
            else if (c_l) P_Ctrl = 1;
            else if (c_r) P_Ctrl = 2;

            int P_Alt = 0;
            bool a_l = (mods & 4);
            bool a_r = (mods & 64);
            if (a_l && a_r) P_Alt = 4;
            else if (a_l) P_Alt = 1;
            else if (a_r) P_Alt = 2;

            int P_Win = 0;
            bool w_l = (mods & 8);
            bool w_r = (mods & 128);
            bool w_apps = (mods & 256);
            if (w_apps) P_Win = 5;
            else if (w_l && w_r) P_Win = 4;
            else if (w_l) P_Win = 1;
            else if (w_r) P_Win = 2;

            int shift_states[4], ctrl_states[4], alt_states[4], win_states[5];
            int shift_cnt = 0, ctrl_cnt = 0, alt_cnt = 0, win_cnt = 0;

            if (P_Shift == 0) shift_states[shift_cnt++] = 0;
            else if (P_Shift == 1) { shift_states[shift_cnt++] = 1; shift_states[shift_cnt++] = 3; }
            else if (P_Shift == 2) { shift_states[shift_cnt++] = 2; shift_states[shift_cnt++] = 3; }
            else if (P_Shift == 4) { shift_states[shift_cnt++] = 4; shift_states[shift_cnt++] = 3; shift_states[shift_cnt++] = 1; shift_states[shift_cnt++] = 2; }

            if (P_Ctrl == 0) ctrl_states[ctrl_cnt++] = 0;
            else if (P_Ctrl == 1) { ctrl_states[ctrl_cnt++] = 1; ctrl_states[ctrl_cnt++] = 3; }
            else if (P_Ctrl == 2) { ctrl_states[ctrl_cnt++] = 2; ctrl_states[ctrl_cnt++] = 3; }
            else if (P_Ctrl == 4) { ctrl_states[ctrl_cnt++] = 4; ctrl_states[ctrl_cnt++] = 3; ctrl_states[ctrl_cnt++] = 1; ctrl_states[ctrl_cnt++] = 2; }

            if (P_Alt == 0) alt_states[alt_cnt++] = 0;
            else if (P_Alt == 1) { alt_states[alt_cnt++] = 1; alt_states[alt_cnt++] = 3; }
            else if (P_Alt == 2) { alt_states[alt_cnt++] = 2; alt_states[alt_cnt++] = 3; }
            else if (P_Alt == 4) { alt_states[alt_cnt++] = 4; alt_states[alt_cnt++] = 3; alt_states[alt_cnt++] = 1; alt_states[alt_cnt++] = 2; }

            if (P_Win == 0) win_states[win_cnt++] = 0;
            else if (P_Win == 1) { win_states[win_cnt++] = 1; win_states[win_cnt++] = 3; }
            else if (P_Win == 2) { win_states[win_cnt++] = 2; win_states[win_cnt++] = 3; }
            else if (P_Win == 4) { win_states[win_cnt++] = 4; win_states[win_cnt++] = 3; win_states[win_cnt++] = 1; win_states[win_cnt++] = 2; }
            else if (P_Win == 5) { win_states[win_cnt++] = 5; }

            for (int w = 0; w < win_cnt; w++) {
                for (int a = 0; a < alt_cnt; a++) {
                    for (int c = 0; c < ctrl_cnt; c++) {
                        for (int s = 0; s < shift_cnt; s++) {
                            int test_idx = F_key + (shift_states[s] * 20) + (ctrl_states[c] * 100) + (alt_states[a] * 500) + (win_states[w] * 2500);
                            if (test_idx >= 1 && test_idx < 15100 && vm->key_state[test_idx] > 0) {
                                idx = test_idx;
                                break;
                            }
                        }
                        if (idx != -1) break;
                    }
                    if (idx != -1) break;
                }
                if (idx != -1) break;
            }
        } else {
            for (int k = 1; k < 15100; ++k) {
                if (vm->key_state[k] > 0 && vm->key_code[k] == key_pressed) {
                    idx = k;
                    break;
                }
            }
        }

        if (idx != -1) {
            if (vm->key_state[idx] == 1) {
                if (!vm->in_key_handler[idx] && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                    vm->in_key_handler[idx] = true;
                    if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                        vm_jump(vm, vm->key_gosub_line[idx], NULL);
                        return;
                    }
                }
            } else if (vm->key_state[idx] == 2) {
                vm->key_pending[idx] = true;
            }
        }
    }

    for (int idx = 1; idx < 15100; ++idx) {
        if (vm->key_state[idx] == 1 && vm->key_pending[idx]) {
            if (!vm->in_key_handler[idx] && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                vm->key_pending[idx] = false;
                vm->in_key_handler[idx] = true;
                if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                    vm_jump(vm, vm->key_gosub_line[idx], NULL);
                    return;
                }
            }
        }
    }

    /* 3. Play Check */
#ifndef BPP_LITE_BUILD
    int current_notes = vdev_music_note_count();
    if (vm->play_state == 1 && vm->play_note_threshold > 0) {
        if (current_notes < vm->play_note_threshold) {
            if (!vm->in_play_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                vm->in_play_handler = true;
                if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                    vm_jump(vm, vm->play_gosub_line, NULL);
                    return;
                }
            }
        }
    } else if (vm->play_state == 2 && vm->play_note_threshold > 0) {
        if (current_notes < vm->play_note_threshold) {
            vm->play_pending = true;
        }
    }

    if (vm->play_state == 1 && vm->play_pending) {
        if (current_notes < vm->play_note_threshold) {
            if (!vm->in_play_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
                vm->play_pending = false;
                vm->in_play_handler = true;
                if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                    vm_jump(vm, vm->play_gosub_line, NULL);
                    return;
                }
            }
        }
    }
#endif

    /* 4. Alarm Countdown Check */
    if (vm->alarm_state == 1 && !vm->in_alarm_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
        double now = platform_get_uptime();
        for (int i = 0; i < vm->alarm_count; i++) {
            BppAlarmCountdown *alarm = &vm->alarms[i];
            if (alarm->state == 1) {
                double diff = now - alarm->last_update_time;
                if (diff > 0.0) {
                    alarm->remaining_seconds -= diff;
                    alarm->last_update_time = now;
                }
                if (alarm->remaining_seconds <= 0.0) {
                    alarm->remaining_seconds = 0.0;
                    alarm->state = 0; /* one-shot */
                    
                    BppLineNumber target_line = alarm->gosub_line;
                    if (target_line == 0.0) {
                        target_line = vm->alarm_gosub_line;
                    }
                    if (target_line > 0.0) {
                        vm->in_alarm_handler = true;
                        if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                            vm_jump(vm, target_line, NULL);
                            return;
                        }
                    }
                }
            }
        }
    } else if (vm->alarm_state == 2 || vm->in_alarm_handler || vm->in_error_handler || try_stack_count(vm->try_stack) > 0) {
        double now = platform_get_uptime();
        for (int i = 0; i < vm->alarm_count; i++) {
            if (vm->alarms[i].state == 1) {
                vm->alarms[i].last_update_time = now;
            }
        }
    }

    /* 5. Alarm Daily Check */
    if (vm->alarm_str_state == 1 && !vm->in_alarm_str_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
        time_t t = time(NULL);
        struct tm tm_buf;
        struct tm *lt = platform_localtime(&t, &tm_buf);
        if (lt) {
            char current_date[64];
            snprintf(current_date, sizeof(current_date), "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
            int current_secs = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
            
            for (int i = 0; i < vm->alarm_str_count; i++) {
                BppAlarmDaily *alarm = &vm->alarms_str[i];
                if (alarm->state == 1) {
                    bool trigger = false;
                    if (alarm->snooze_trigger_time > 0) {
                        if (current_secs >= alarm->snooze_trigger_time) {
                            trigger = true;
                            alarm->snooze_trigger_time = -1;
                            strncpy(alarm->last_triggered_date, current_date, sizeof(alarm->last_triggered_date) - 1);
                            alarm->last_triggered_date[sizeof(alarm->last_triggered_date) - 1] = '\0';
                        }
                    } else {
                        if (strcmp(alarm->last_triggered_date, current_date) != 0) {
                            if (current_secs >= alarm->seconds_since_midnight) {
                                trigger = true;
                                strncpy(alarm->last_triggered_date, current_date, sizeof(alarm->last_triggered_date) - 1);
                                alarm->last_triggered_date[sizeof(alarm->last_triggered_date) - 1] = '\0';
                            }
                        }
                    }
                    
                    if (trigger) {
                        BppLineNumber target_line = alarm->gosub_line;
                        if (target_line == 0.0) {
                            target_line = vm->alarm_str_gosub_line;
                        }
                        if (target_line > 0.0) {
                            vm->in_alarm_str_handler = true;
                            if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                                vm_jump(vm, target_line, NULL);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    /* 6. Mouse Event Trapping */
    int m_col = 0, m_row = 0;
    platform_mouse_get_position(&m_col, &m_row);
    
    int m_btn_mask = 0;
    for (int b = 0; b < 3; b++) {
        if (platform_mouse_get_button(b) != 0) {
            m_btn_mask |= (1 << b);
        }
    }

    bool pos_changed = (m_col != vm->last_mouse_col || m_row != vm->last_mouse_row);
    if (pos_changed) {
        console_hide_mouse_cursor();
        console_draw_mouse_cursor();
    }
    bool btn_changed = (m_btn_mask != vm->last_mouse_btn_mask);
    double now = platform_get_uptime();

    if (btn_changed) {
        for (int b = 0; b < 3; b++) {
            int old_pressed = (vm->last_mouse_btn_mask >> b) & 1;
            int new_pressed = (m_btn_mask >> b) & 1;
            if (new_pressed && !old_pressed) {
                vm->last_mouse_click_btn = b;
                if (now - vm->last_click_time < 0.25) {
                    vm->last_mouse_click_type++;
                    if (vm->last_mouse_click_type > 3) vm->last_mouse_click_type = 3;
                } else {
                    vm->last_mouse_click_type = 1;
                }
                vm->last_click_time = now;
            }
        }
    }

    /* A. ON TRIG GOSUB trapping */
    if (vm->trig_state == 1 && btn_changed && !vm->in_trig_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
        bool trigger = false;
        if (vm->trig_target_btn == -1) {
            for (int b = 0; b < 3; b++) {
                int old_pressed = (vm->last_mouse_btn_mask >> b) & 1;
                int new_pressed = (m_btn_mask >> b) & 1;
                if (new_pressed && !old_pressed) trigger = true;
            }
        } else {
            int old_pressed = (vm->last_mouse_btn_mask >> vm->trig_target_btn) & 1;
            int new_pressed = (m_btn_mask >> vm->trig_target_btn) & 1;
            if (new_pressed && !old_pressed) trigger = true;
        }
        
        if (trigger && vm->trig_gosub_line > 0.0) {
            vm->in_trig_handler = true;
            vm->last_mouse_btn_mask = m_btn_mask;
            if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                vm_jump(vm, vm->trig_gosub_line, NULL);
                return;
            }
        }
    }

    /* B. ON HMOUSE GOSUB */
    if (vm->hmouse_state == 1 && m_col != vm->last_mouse_col && !vm->in_hmouse_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
        if (vm->hmouse_gosub_line > 0.0) {
            vm->in_hmouse_handler = true;
            vm->last_mouse_col = m_col;
            if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                vm_jump(vm, vm->hmouse_gosub_line, NULL);
                return;
            }
        }
    }

    /* C. ON VMOUSE GOSUB */
    if (vm->vmouse_state == 1 && m_row != vm->last_mouse_row && !vm->in_vmouse_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
        if (vm->vmouse_gosub_line > 0.0) {
            vm->in_vmouse_handler = true;
            vm->last_mouse_row = m_row;
            if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                vm_jump(vm, vm->vmouse_gosub_line, NULL);
                return;
            }
        }
    }

    /* D. ON MOUSE GOSUB */
    if (vm->mouse_state == 1 && pos_changed && !vm->in_mouse_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
        int hover_char = 32;
        VConContext *vcon = vm->vcon;
        if (vcon) {
            int active_idx = vcon_get_active_index(vcon);
            hover_char = vcon_get_char_at(vcon, active_idx, m_row - 1, m_col - 1);
        }
        
        bool trigger = false;
        if (vm->mouse_target_char == -1) {
            trigger = true;
        } else if (vm->mouse_target_char == hover_char) {
            trigger = true;
        }
        
        if (trigger && vm->mouse_gosub_line > 0.0) {
            vm->in_mouse_handler = true;
            vm->last_mouse_col = m_col;
            vm->last_mouse_row = m_row;
            if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
                vm_jump(vm, vm->mouse_gosub_line, NULL);
                return;
            }
        }
    }

    vm->last_mouse_col = m_col;
    vm->last_mouse_row = m_row;
    vm->last_mouse_btn_mask = m_btn_mask;
}

void vm_clear_event_handlers(VMContext *vm) {
    if (!vm) return;
    vm->in_timer_handler = false;
    for (int i = 1; i < 15100; ++i) {
        vm->in_key_handler[i] = false;
    }
    vm->in_play_handler = false;
    vm->in_alarm_handler = false;
    vm->in_alarm_str_handler = false;
    vm->in_mouse_handler = false;
    vm->in_hmouse_handler = false;
    vm->in_vmouse_handler = false;
    vm->in_trig_handler = false;
}


static bool parse_time_str(const char *time_str, int *out_secs) {
    if (!time_str) return false;
    int hh = 0, mm = 0, ss = 0;
    
    if (sscanf(time_str, "%d:%d:%d", &hh, &mm, &ss) != 3) {
        return false;
    }
    
    if (mm < 0 || mm > 59 || ss < 0 || ss > 59) {
        return false;
    }
    
    const char *p = time_str;
    while (*p && (*p == ' ' || isdigit((unsigned char)*p) || *p == ':')) {
        p++;
    }
    
    if (*p) {
        char suffix[16] = "";
        int len = 0;
        while (*p && len < 15) {
            suffix[len++] = (char)toupper((unsigned char)*p);
            p++;
        }
        suffix[len] = '\0';
        
        if (hh < 1 || hh > 12) {
            return false;
        }
        
        if (strcmp(suffix, "PM") == 0 || strcmp(suffix, "P") == 0) {
            if (hh < 12) {
                hh += 12;
            }
        } else if (strcmp(suffix, "AM") == 0 || strcmp(suffix, "A") == 0) {
            if (hh == 12) {
                hh = 0;
            }
        } else {
            return false;
        }
    } else {
        if (hh < 0 || hh > 23) {
            return false;
        }
    }
    
    if (out_secs) {
        *out_secs = hh * 3600 + mm * 60 + ss;
    }
    return true;
}

bool vm_validate_time_str(const char *time_str, int *out_secs) {
    return parse_time_str(time_str, out_secs);
}

static bool grow_alarms(VMContext *vm) {
    int new_cap = vm->alarm_count + 1;
    BppAlarmCountdown *new_arr = (BppAlarmCountdown *)realloc(vm->alarms, new_cap * sizeof(BppAlarmCountdown));
    if (!new_arr) return false;
    vm->alarms = new_arr;
    memset(&vm->alarms[vm->alarm_count], 0, sizeof(BppAlarmCountdown));
    vm->alarm_count = new_cap;
    return true;
}

static bool grow_alarms_str(VMContext *vm) {
    int new_cap = vm->alarm_str_count + 1;
    BppAlarmDaily *new_arr = (BppAlarmDaily *)realloc(vm->alarms_str, new_cap * sizeof(BppAlarmDaily));
    if (!new_arr) return false;
    vm->alarms_str = new_arr;
    memset(&vm->alarms_str[vm->alarm_str_count], 0, sizeof(BppAlarmDaily));
    vm->alarms_str[vm->alarm_str_count].snooze_trigger_time = -1;
    vm->alarm_str_count = new_cap;
    return true;
}

void vm_set_alarm_countdown_trap(VMContext *vm, double seconds, BppLineNumber line) {
    if (!vm) return;
    if (seconds < 0.0) {
        vm->alarm_gosub_line = line;
        return;
    }
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].initial_seconds == seconds) {
            vm->alarms[i].gosub_line = line;
            return;
        }
    }
    if (grow_alarms(vm)) {
        BppAlarmCountdown *alarm = &vm->alarms[vm->alarm_count - 1];
        alarm->initial_seconds = seconds;
        alarm->remaining_seconds = seconds;
        alarm->state = 0;
        alarm->last_update_time = platform_get_uptime();
        alarm->gosub_line = line;
    }
}

void vm_set_alarm_countdown_state(VMContext *vm, double seconds, int state) {
    if (!vm || seconds < 1.0 || seconds > 86400.0) return;
    
    int idx = -1;
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].initial_seconds == seconds) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1 && (state == 1 || state == 2)) {
        if (grow_alarms(vm)) {
            idx = vm->alarm_count - 1;
            vm->alarms[idx].initial_seconds = seconds;
            vm->alarms[idx].remaining_seconds = seconds;
        } else {
            return;
        }
    }
    
    if (idx != -1) {
        BppAlarmCountdown *alarm = &vm->alarms[idx];
        if (state == 1) {
            if (alarm->remaining_seconds <= 0.0) {
                alarm->remaining_seconds = alarm->initial_seconds;
            }
            alarm->last_update_time = platform_get_uptime();
        }
        alarm->state = state;
    }
}

void vm_set_alarm_countdown_remaining(VMContext *vm, double seconds, double remaining) {
    if (!vm) return;
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].initial_seconds == seconds) {
            vm->alarms[i].remaining_seconds = remaining;
            vm->alarms[i].last_update_time = platform_get_uptime();
            break;
        }
    }
}

double vm_get_alarm_countdown(VMContext *vm, double seconds) {
    if (!vm) return 0.0;
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].initial_seconds == seconds) {
            return vm->alarms[i].remaining_seconds;
        }
    }
    return 0.0;
}

double vm_get_closest_alarm_countdown(VMContext *vm) {
    if (!vm) return 0.0;
    double min_val = -1.0;
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].state == 1) {
            if (min_val < 0.0 || vm->alarms[i].remaining_seconds < min_val) {
                min_val = vm->alarms[i].remaining_seconds;
            }
        }
    }
    return (min_val < 0.0) ? 0.0 : min_val;
}

void vm_set_alarm_daily_trap(VMContext *vm, const char *time_str, BppLineNumber line) {
    if (!vm) return;
    if (!time_str) {
        vm->alarm_str_gosub_line = line;
        return;
    }
    
    int secs = 0;
    if (!parse_time_str(time_str, &secs)) {
        return;
    }
    
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            vm->alarms_str[i].gosub_line = line;
            return;
        }
    }
    
    if (grow_alarms_str(vm)) {
        BppAlarmDaily *alarm = &vm->alarms_str[vm->alarm_str_count - 1];
        strncpy(alarm->target_time, time_str, sizeof(alarm->target_time) - 1);
        alarm->target_time[sizeof(alarm->target_time) - 1] = '\0';
        alarm->state = 0;
        alarm->seconds_since_midnight = secs;
        alarm->gosub_line = line;
        alarm->last_triggered_date[0] = '\0';
    }
}

void vm_set_alarm_daily_state(VMContext *vm, const char *time_str, int state) {
    if (!vm || !time_str) return;
    
    int secs = 0;
    if (!parse_time_str(time_str, &secs)) {
        return;
    }
    
    int idx = -1;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1 && (state == 1 || state == 2)) {
        if (grow_alarms_str(vm)) {
            idx = vm->alarm_str_count - 1;
            strncpy(vm->alarms_str[idx].target_time, time_str, sizeof(vm->alarms_str[idx].target_time) - 1);
            vm->alarms_str[idx].target_time[sizeof(vm->alarms_str[idx].target_time) - 1] = '\0';
            vm->alarms_str[idx].seconds_since_midnight = secs;
            vm->alarms_str[idx].last_triggered_date[0] = '\0';
        } else {
            return;
        }
    }
    
    if (idx != -1) {
        BppAlarmDaily *alarm = &vm->alarms_str[idx];
        if (state == 1) {
            time_t t = time(NULL);
            struct tm tm_buf;
            struct tm *lt = platform_localtime(&t, &tm_buf);
            if (lt) {
                char current_date[64];
                snprintf(current_date, sizeof(current_date), "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
                int current_secs = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
                if (current_secs >= alarm->seconds_since_midnight) {
                    strncpy(alarm->last_triggered_date, current_date, sizeof(alarm->last_triggered_date) - 1);
                    alarm->last_triggered_date[sizeof(alarm->last_triggered_date) - 1] = '\0';
                } else {
                    alarm->last_triggered_date[0] = '\0';
                }
            }
        }
        alarm->state = state;
    }
}

double vm_get_alarm_daily_remaining(VMContext *vm, const char *time_str) {
    if (!vm || !time_str) return -1.0;
    
    int idx = -1;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1) return -1.0;
    
    time_t t = time(NULL);
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);
    if (!lt) return -1.0;
    
    int current_secs = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
    int target_secs = vm->alarms_str[idx].seconds_since_midnight;
    if (vm->alarms_str[idx].snooze_trigger_time > 0) {
        target_secs = vm->alarms_str[idx].snooze_trigger_time;
    }
    
    int diff = target_secs - current_secs;
    if (diff < 0) {
        diff += 86400;
    }
    return (double)diff;
}

double vm_get_closest_alarm_daily_remaining(VMContext *vm) {
    if (!vm) return -1.0;
    double min_val = -1.0;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (vm->alarms_str[i].state == 1) {
            double rem = vm_get_alarm_daily_remaining(vm, vm->alarms_str[i].target_time);
            if (rem >= 0.0) {
                if (min_val < 0.0 || rem < min_val) {
                    min_val = rem;
                }
            }
        }
    }
    return min_val;
}

void vm_set_global_alarm_state(VMContext *vm, int state) {
    if (!vm) return;
    vm->alarm_state = state;
    if (state == 1) {
        double now = platform_get_uptime();
        for (int i = 0; i < vm->alarm_count; i++) {
            if (vm->alarms[i].state == 1) {
                vm->alarms[i].last_update_time = now;
            }
        }
    }
}

void vm_set_global_alarm_str_state(VMContext *vm, int state) {
    if (!vm) return;
    vm->alarm_str_state = state;
}

void OBSOLETE_vm_with_stack_push(VMContext *vm, const char *path) {
    if (vm && vm->with_stack_depth < 8) {
        strncpy(vm->with_stack[vm->with_stack_depth++], path, 255);
        vm->with_stack[vm->with_stack_depth - 1][255] = '\0';
    }
}

void OBSOLETE_vm_with_stack_pop(VMContext *vm) {
    if (vm && vm->with_stack_depth > 0) {
        vm->with_stack_depth--;
    }
}

void OBSOLETE_vm_with_stack_clear(VMContext *vm) {
    if (vm) {
        vm->with_stack_depth = 0;
    }
}

const char *OBSOLETE_vm_with_stack_peek(VMContext *vm) {
    if (vm && vm->with_stack_depth > 0) {
        return vm->with_stack[vm->with_stack_depth - 1];
    }
    return NULL;
}

void vm_snooze_alarm_countdown(VMContext *vm, double seconds, double snooze_amount) {
    if (!vm || seconds < 1.0 || seconds > 86400.0) return;
    int idx = -1;
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].initial_seconds == seconds) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        if (grow_alarms(vm)) {
            idx = vm->alarm_count - 1;
            vm->alarms[idx].initial_seconds = seconds;
            vm->alarms[idx].remaining_seconds = seconds;
        } else {
            return;
        }
    }
    BppAlarmCountdown *alarm = &vm->alarms[idx];
    if (alarm->remaining_seconds <= 0.0) {
        alarm->remaining_seconds = snooze_amount;
    } else {
        alarm->remaining_seconds += snooze_amount;
    }
    alarm->last_update_time = platform_get_uptime();
    alarm->state = 1; /* Always ON when snoozed */
}

void vm_unset_alarm_countdown(VMContext *vm, double seconds) {
    if (!vm) return;
    int idx = -1;
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].initial_seconds == seconds) {
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        for (int i = idx; i < vm->alarm_count - 1; i++) {
            vm->alarms[i] = vm->alarms[i + 1];
        }
        vm->alarm_count--;
    }
}

void vm_snooze_alarm_daily(VMContext *vm, const char *time_str, int snooze_amount) {
    if (!vm || !time_str) return;
    int secs = 0;
    if (!vm_validate_time_str(time_str, &secs)) return;
    
    int idx = -1;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        if (grow_alarms_str(vm)) {
            idx = vm->alarm_str_count - 1;
            strncpy(vm->alarms_str[idx].target_time, time_str, sizeof(vm->alarms_str[idx].target_time) - 1);
            vm->alarms_str[idx].target_time[sizeof(vm->alarms_str[idx].target_time) - 1] = '\0';
            vm->alarms_str[idx].seconds_since_midnight = secs;
            vm->alarms_str[idx].last_triggered_date[0] = '\0';
            vm->alarms_str[idx].snooze_trigger_time = -1;
        } else {
            return;
        }
    }
    
    time_t t = time(NULL);
    struct tm tm_buf;
    struct tm *lt = platform_localtime(&t, &tm_buf);
    if (lt) {
        int current_secs = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
        vm->alarms_str[idx].snooze_trigger_time = (current_secs + snooze_amount) % 86400;
        vm->alarms_str[idx].state = 1; /* Always ON when snoozed */
    }
}

void vm_unset_alarm_daily(VMContext *vm, const char *time_str) {
    if (!vm || !time_str) return;
    int idx = -1;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        for (int i = idx; i < vm->alarm_str_count - 1; i++) {
            vm->alarms_str[i] = vm->alarms_str[i + 1];
        }
        vm->alarm_str_count--;
    }
}

void vm_snooze_all_countdowns(VMContext *vm, double snooze_amount) {
    if (!vm) return;
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].state == 1) {
            vm_snooze_alarm_countdown(vm, vm->alarms[i].initial_seconds, snooze_amount);
        }
    }
}

void vm_snooze_all_dailies(VMContext *vm, int snooze_amount) {
    if (!vm) return;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (vm->alarms_str[i].state == 1) {
            vm_snooze_alarm_daily(vm, vm->alarms_str[i].target_time, snooze_amount);

}
}
}
