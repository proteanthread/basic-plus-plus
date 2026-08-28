// FILENAME: interrupt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (call.c, exec_internal.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (bios.h, bios.c, eval.h, eval.c, interrupt.h, map.h, map.c)
// NEEDS: libengine (string.c, time.h, time.c)
// Provides runtime implementation for the INTERRUPT statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/interrupt.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include "runtime/micro_lib_metadata.h"
#include "bios/bios.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif

static void get_struct_reg(VMContext *vm, const char *prefix, const char *field, uint16_t *out_reg) {
    VariableContext *vc = vm_get_var(vm);
    // 1. Try Map lookup on base variable
    BValue *base_val = var_lookup(vc, prefix, false);
    if (base_val && base_val->type == VAL_MAP && base_val->as.map) {
        BValue fval;
        if (map_get(base_val->as.map, field, &fval)) {
            *out_reg = (uint16_t)((long)fval.as.number & 0xFFFF);
            return;
        }
        char upper_field[32];
        size_t flen = strlen(field);
        for (size_t i = 0; i <= flen; i++) upper_field[i] = (char)toupper((unsigned char)field[i]);
        if (map_get(base_val->as.map, upper_field, &fval)) {
            *out_reg = (uint16_t)((long)fval.as.number & 0xFFFF);
            return;
        }
    }
    // 2. Flat lookup "prefix.field"
    char varname[128];
    snprintf(varname, sizeof(varname), "%s.%s", prefix, field);
    BValue *val = var_lookup(vc, varname, false);
    if (val && (val->type == VAL_NUMBER || val->type == VAL_INTEGER)) {
        *out_reg = (uint16_t)((long)val->as.number & 0xFFFF);
    }
}

static void set_struct_reg(VMContext *vm, const char *prefix, const char *field, uint16_t reg_val) {
    VariableContext *vc = vm_get_var(vm);
    BValue val = { .type = VAL_NUMBER, .as.number = (double)reg_val };

    // 1. Set on Base Map
    BValue *base_val = var_lookup(vc, prefix, false);
    if (!base_val || base_val->type != VAL_MAP || !base_val->as.map) {
        BppMap *new_map = map_create();
        if (new_map) {
            BValue map_val = { .type = VAL_MAP, .as.map = new_map };
            var_assign(vc, prefix, map_val);
            base_val = var_lookup(vc, prefix, false);
        }
    }
    if (base_val && base_val->type == VAL_MAP && base_val->as.map) {
        map_set(vm_get_str(vm), base_val->as.map, field, val);
    }

    // 2. Set flat variable "prefix.field"
    char varname[128];
    snprintf(varname, sizeof(varname), "%s.%s", prefix, field);
    var_assign(vc, varname, val);
}

static BppError execute_interrupt(VMContext *vm, uint8_t int_num, const char *in_name, const char *out_name, bool is_extended) {
    BppError err; memset(&err, 0, sizeof(err));
    BiosRegs regs;
    memset(&regs, 0, sizeof(regs));

    // Unpack input registers
    get_struct_reg(vm, in_name, "ax", &regs.ax);
    get_struct_reg(vm, in_name, "bx", &regs.bx);
    get_struct_reg(vm, in_name, "cx", &regs.cx);
    get_struct_reg(vm, in_name, "dx", &regs.dx);
    get_struct_reg(vm, in_name, "bp", &regs.bp);
    get_struct_reg(vm, in_name, "si", &regs.si);
    get_struct_reg(vm, in_name, "di", &regs.di);
    get_struct_reg(vm, in_name, "flags", &regs.flags);
    if (is_extended) {
        get_struct_reg(vm, in_name, "ds", &regs.ds);
        get_struct_reg(vm, in_name, "es", &regs.es);
    }

    uint8_t ah = (uint8_t)((regs.ax >> 8) & 0xFF);

    // DOS / BIOS Interrupt Emulation Services
    if (int_num == 0x21) {
        // DOS Services
        if (ah == 0x30) {
            // Get DOS Version: Major=5 (MS-DOS 5.00), Minor=0
            regs.ax = (0x00 << 8) | 0x05;
            regs.bx = 0xFF00; // Serial
            regs.cx = 0x0000;
        } else if (ah == 0x2A) {
            // Get Date: CX=year, DH=month, DL=day, AL=day of week
            time_t rawtime;
            time(&rawtime);
            struct tm *ti = localtime(&rawtime);
            if (ti) {
                regs.cx = (uint16_t)(ti->tm_year + 1900);
                regs.dx = (uint16_t)(((ti->tm_mon + 1) << 8) | ti->tm_mday);
                regs.ax = (uint16_t)((regs.ax & 0xFF00) | (ti->tm_wday));
            }
        } else if (ah == 0x2C) {
            // Get Time: CH=hour, CL=minute, DH=second, DL=hundredths
            time_t rawtime;
            time(&rawtime);
            struct tm *ti = localtime(&rawtime);
            if (ti) {
                regs.cx = (uint16_t)((ti->tm_hour << 8) | ti->tm_min);
                regs.dx = (uint16_t)((ti->tm_sec << 8) | 0);
            }
        }
    } else if (int_num == 0x1A) {
        // BIOS Clock Services
        if (ah == 0x00) {
            // Get System Clock Ticks since midnight (18.2 Hz)
            time_t rawtime;
            time(&rawtime);
            struct tm *ti = localtime(&rawtime);
            if (ti) {
                long sec_midnight = ti->tm_hour * 3600 + ti->tm_min * 60 + ti->tm_sec;
                long ticks = (long)(sec_midnight * 18.2065);
                regs.cx = (uint16_t)((ticks >> 16) & 0xFFFF);
                regs.dx = (uint16_t)(ticks & 0xFFFF);
                regs.ax = 0; // Midnight flag
            }
        }
    } else if (int_num == 0x10) {
        // BIOS Video Services
        if (ah == 0x0F) {
            // Get Video Mode: AL=mode (3=80x25 text), AH=columns (80), BH=active page (0)
            regs.ax = (80 << 8) | 0x03;
            regs.bx = 0x0000;
        }
    }

    // Write back updated registers to output struct
    set_struct_reg(vm, out_name, "ax", regs.ax);
    set_struct_reg(vm, out_name, "bx", regs.bx);
    set_struct_reg(vm, out_name, "cx", regs.cx);
    set_struct_reg(vm, out_name, "dx", regs.dx);
    set_struct_reg(vm, out_name, "bp", regs.bp);
    set_struct_reg(vm, out_name, "si", regs.si);
    set_struct_reg(vm, out_name, "di", regs.di);
    set_struct_reg(vm, out_name, "flags", regs.flags);
    if (is_extended) {
        set_struct_reg(vm, out_name, "ds", regs.ds);
        set_struct_reg(vm, out_name, "es", regs.es);
    }

    return err;
}

static BppError parse_interrupt_args(VMContext *vm, LexerContext *lex, bool is_extended) {
    BppError err; memset(&err, 0, sizeof(err));
    bool has_lparen = false;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
        has_lparen = true;
    }

    // 1. Interrupt Number
    BValue val_int = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    uint8_t int_num = (uint8_t)((int)val_int.as.number & 0xFF);

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected comma after interrupt number"; return err; }

    // 2. InRegs Struct Variable
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) { err.code = 2; err.message = "Expected input register variable"; return err; }
    char in_name[64] = {0};
    size_t len = (tok.length < 63) ? tok.length : 63;
    memcpy(in_name, tok.start, len);

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected comma after inregs"; return err; }

    // 3. OutRegs Struct Variable
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) { err.code = 2; err.message = "Expected output register variable"; return err; }
    char out_name[64] = {0};
    len = (tok.length < 63) ? tok.length : 63;
    memcpy(out_name, tok.start, len);

    if (has_lparen) {
        tok = lex_next(lex);
        if (tok.type != TOK_RPAREN) { err.code = 2; err.message = "Expected closing parenthesis"; return err; }
    }

    return execute_interrupt(vm, int_num, in_name, out_name, is_extended);
}

BppError stmt_interrupt_handler(VMContext *vm, LexerContext *lex) {
    return parse_interrupt_args(vm, lex, false);
}

BppError stmt_interruptx_handler(VMContext *vm, LexerContext *lex) {
    return parse_interrupt_args(vm, lex, true);
}

void stmt_interrupt_register(void) {
    static const MicroLibMetadata meta = {
        .name = "INTERRUPT",
        .category = "System & Hardware",
        .syntax = "CALL INTERRUPT(int_num%, inregs, outregs) / INTERRUPT int_num%, inregs, outregs"
    };
    microlib_register(&meta);
}
