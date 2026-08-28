// FILENAME: events_poll.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (events_internal.h, events_net.h, events_net.c)
// Implements bytecode virtual machine execution and state for events_poll.
//
// ---- Includes ----

#include "vm/events_internal.h"
#include "vm/events_net.h"

//
// ---- Event Polling Loop ----

void vm_trigger_event_polling(VMContext *vm) {
    if (!vm || !vm->running) return;

    // 1. Timer Check
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

    // 2. Key Check
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

    // 3. Play Check
#ifndef BASIC_LITE_BUILD
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

    // 4. Alarm Countdown Check
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
                    alarm->state = 0;

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

    // 5. Alarm Daily Check
    if (vm->alarm_str_state == 1 && !vm->in_alarm_str_handler && !vm->in_error_handler && try_stack_count(vm->try_stack) == 0) {
        HalContext *hal = hal_get();
        time_t t = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
        struct tm tm_buf;

        struct tm *lt = platform_localtime(&t, &tm_buf);
        if (lt) {
            char current_date[64];
            runtime_snprintf(current_date, sizeof(current_date), "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
            int current_secs = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;

            for (int i = 0; i < vm->alarm_str_count; i++) {
                BppAlarmDaily *alarm = &vm->alarms_str[i];
                if (alarm->state == 1) {
                    bool trigger = false;
                    if (alarm->snooze_trigger_time > 0) {
                        if (current_secs >= alarm->snooze_trigger_time) {
                            trigger = true;
                            alarm->snooze_trigger_time = -1;
                            runtime_strncpy(alarm->last_triggered_date, current_date, sizeof(alarm->last_triggered_date) - 1);
                            alarm->last_triggered_date[sizeof(alarm->last_triggered_date) - 1] = '\0';
                        }
                    } else {
                        if (runtime_strcmp(alarm->last_triggered_date, current_date) != 0) {
                            if (current_secs >= alarm->seconds_since_midnight) {
                                trigger = true;
                                runtime_strncpy(alarm->last_triggered_date, current_date, sizeof(alarm->last_triggered_date) - 1);
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

    // 6. Mouse Event Trapping
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

    // A. ON TRIG GOSUB trapping
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

    // B. ON HMOUSE GOSUB
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

    // C. ON VMOUSE GOSUB
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

    // D. ON MOUSE GOSUB
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

    // E. ON NET / ON PORT / ON PEER / ON SNIFF GOSUB
    BppLineNumber net_line = 0;
    if (vm_check_net_events(vm, &net_line) && net_line > 0.0 && !vm->in_error_handler) {
        if (vm_gosub_push(vm, vm->current_line, vm->current_pos)) {
            vm_jump(vm, net_line, NULL);
            return;
        }
    }
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
