// FILENAME: events_alarm.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (events_internal.h)
// Implements bytecode virtual machine execution and state for events_alarm.
//
// ---- Includes ----

#include "vm/events_internal.h"

//
// ---- Time String Parsing ----

bool parse_time_str(const char *time_str, int *out_secs) {
    if (!time_str) return false;
    const char *p = time_str;
    while (*p && runtime_isspace((unsigned char)*p)) p++;

    if (!runtime_isdigit((unsigned char)*p)) return false;
    int hh = 0;
    while (*p && runtime_isdigit((unsigned char)*p)) {
        hh = hh * 10 + (*p - '0');
        p++;
    }
    if (*p != ':') return false;
    p++; // skip ':'

    if (!runtime_isdigit((unsigned char)*p)) return false;
    int mm = 0;
    while (*p && runtime_isdigit((unsigned char)*p)) {
        mm = mm * 10 + (*p - '0');
        p++;
    }
    if (*p != ':') return false;
    p++; // skip ':'

    if (!runtime_isdigit((unsigned char)*p)) return false;
    int ss = 0;
    while (*p && runtime_isdigit((unsigned char)*p)) {
        ss = ss * 10 + (*p - '0');
        p++;
    }

    if (mm < 0 || mm > 59 || ss < 0 || ss > 59) {
        return false;
    }

    while (*p && runtime_isspace((unsigned char)*p)) {
        p++;
    }

    if (*p) {
        char suffix[16] = "";
        int len = 0;
        while (*p && len < 15) {
            suffix[len++] = (char)runtime_toupper((unsigned char)*p);
            p++;
        }
        suffix[len] = '\0';

        if (hh < 1 || hh > 12) {
            return false;
        }

        if (runtime_strcmp(suffix, "PM") == 0 || runtime_strcmp(suffix, "P") == 0) {
            if (hh < 12) {
                hh += 12;
            }
        } else if (runtime_strcmp(suffix, "AM") == 0 || runtime_strcmp(suffix, "A") == 0) {
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

//
// ---- Dynamic Alarm Arrays ----

bool grow_alarms(VMContext *vm) {
    HalContext *hal = hal_get();
    int new_cap = vm->alarm_count + 1;
    BppAlarmCountdown *new_arr = NULL;
    if (hal && hal->mem.realloc) {
        new_arr = (BppAlarmCountdown *)hal->mem.realloc(vm->alarms, new_cap * sizeof(BppAlarmCountdown));
    } else if (hal && hal->mem.alloc) {
        new_arr = (BppAlarmCountdown *)hal->mem.alloc(new_cap * sizeof(BppAlarmCountdown));
        if (new_arr && vm->alarms) {
            runtime_memcpy(new_arr, vm->alarms, vm->alarm_count * sizeof(BppAlarmCountdown));
            if (hal->mem.free) hal->mem.free(vm->alarms);
        }
    }
    if (!new_arr) return false;
    vm->alarms = new_arr;
    runtime_memset(&vm->alarms[vm->alarm_count], 0, sizeof(BppAlarmCountdown));
    vm->alarm_count = new_cap;
    return true;
}

bool grow_alarms_str(VMContext *vm) {
    HalContext *hal = hal_get();
    int new_cap = vm->alarm_str_count + 1;
    BppAlarmDaily *new_arr = NULL;
    if (hal && hal->mem.realloc) {
        new_arr = (BppAlarmDaily *)hal->mem.realloc(vm->alarms_str, new_cap * sizeof(BppAlarmDaily));
    } else if (hal && hal->mem.alloc) {
        new_arr = (BppAlarmDaily *)hal->mem.alloc(new_cap * sizeof(BppAlarmDaily));
        if (new_arr && vm->alarms_str) {
            runtime_memcpy(new_arr, vm->alarms_str, vm->alarm_str_count * sizeof(BppAlarmDaily));
            if (hal->mem.free) hal->mem.free(vm->alarms_str);
        }
    }
    if (!new_arr) return false;
    vm->alarms_str = new_arr;
    runtime_memset(&vm->alarms_str[vm->alarm_str_count], 0, sizeof(BppAlarmDaily));
    vm->alarms_str[vm->alarm_str_count].snooze_trigger_time = -1;
    vm->alarm_str_count = new_cap;
    return true;
}

//
// ---- Countdown Alarms ----

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
    alarm->state = 1;
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

void vm_snooze_all_countdowns(VMContext *vm, double snooze_amount) {
    if (!vm) return;
    for (int i = 0; i < vm->alarm_count; i++) {
        if (vm->alarms[i].state == 1) {
            vm_snooze_alarm_countdown(vm, vm->alarms[i].initial_seconds, snooze_amount);
        }
    }
}

//
// ---- Daily Alarms ----

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
        if (runtime_strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            vm->alarms_str[i].gosub_line = line;
            return;
        }
    }

    if (grow_alarms_str(vm)) {
        BppAlarmDaily *alarm = &vm->alarms_str[vm->alarm_str_count - 1];
        runtime_strncpy(alarm->target_time, time_str, sizeof(alarm->target_time) - 1);
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
        if (runtime_strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            idx = i;
            break;
        }
    }

    if (idx == -1 && (state == 1 || state == 2)) {
        if (grow_alarms_str(vm)) {
            idx = vm->alarm_str_count - 1;
            runtime_strncpy(vm->alarms_str[idx].target_time, time_str, sizeof(vm->alarms_str[idx].target_time) - 1);
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
            HalContext *hal = hal_get();
            time_t t = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
            struct tm tm_buf;

            struct tm *lt = platform_localtime(&t, &tm_buf);
            if (lt) {
                char current_date[64];
                runtime_snprintf(current_date, sizeof(current_date), "%04d-%02d-%02d", lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
                int current_secs = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
                if (current_secs >= alarm->seconds_since_midnight) {
                    runtime_strncpy(alarm->last_triggered_date, current_date, sizeof(alarm->last_triggered_date) - 1);
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
        if (runtime_strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return -1.0;

    HalContext *hal = hal_get();
    time_t t = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
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

void vm_set_global_alarm_str_state(VMContext *vm, int state) {
    if (!vm) return;
    vm->alarm_str_state = state;
}

void vm_snooze_alarm_daily(VMContext *vm, const char *time_str, int snooze_amount) {
    if (!vm || !time_str) return;
    int secs = 0;
    if (!vm_validate_time_str(time_str, &secs)) return;

    int idx = -1;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (runtime_strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        if (grow_alarms_str(vm)) {
            idx = vm->alarm_str_count - 1;
            runtime_strncpy(vm->alarms_str[idx].target_time, time_str, sizeof(vm->alarms_str[idx].target_time) - 1);
            vm->alarms_str[idx].target_time[sizeof(vm->alarms_str[idx].target_time) - 1] = '\0';
            vm->alarms_str[idx].seconds_since_midnight = secs;
            vm->alarms_str[idx].last_triggered_date[0] = '\0';
            vm->alarms_str[idx].snooze_trigger_time = -1;
        } else {
            return;
        }
    }

    HalContext *hal = hal_get();
    time_t t = (time_t)(hal && hal->time.now_epoch_seconds ? hal->time.now_epoch_seconds() : 0);
    struct tm tm_buf;

    struct tm *lt = platform_localtime(&t, &tm_buf);
    if (lt) {
        int current_secs = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
        vm->alarms_str[idx].snooze_trigger_time = (current_secs + snooze_amount) % 86400;
        vm->alarms_str[idx].state = 1;
    }
}

void vm_unset_alarm_daily(VMContext *vm, const char *time_str) {
    if (!vm || !time_str) return;
    int idx = -1;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (runtime_strcasecmp(vm->alarms_str[i].target_time, time_str) == 0) {
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

void vm_snooze_all_dailies(VMContext *vm, int snooze_amount) {
    if (!vm) return;
    for (int i = 0; i < vm->alarm_str_count; i++) {
        if (vm->alarms_str[i].state == 1) {
            vm_snooze_alarm_daily(vm, vm->alarms_str[i].target_time, snooze_amount);
        }
    }
}
