// FILENAME: events_trap.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (events_internal.h)
// Implements bytecode virtual machine execution and state for events_trap.
//
// ---- Includes ----

#include "vm/events_internal.h"

//
// ---- Timer Traps ----

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

//
// ---- Key Traps ----

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

//
// ---- USR Function Pointers ----

void vm_set_usr_ptr(VMContext *vm, int idx, uintptr_t ptr) {
    if (!vm || idx < 0 || idx > 9) return;
    vm->usr_pointers[idx] = ptr;
}

uintptr_t vm_get_usr_ptr(const VMContext *vm, int idx) {
    if (!vm || idx < 0 || idx > 9) return 0;
    return vm->usr_pointers[idx];
}

//
// ---- Communication, Pen, Strig, Play, Mouse Traps ----

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
    if (target_char != -2) vm->mouse_target_char = target_char;
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
    if (target_btn != -2) vm->trig_target_btn = target_btn;
}
