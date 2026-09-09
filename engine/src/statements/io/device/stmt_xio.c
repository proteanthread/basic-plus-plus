// FILENAME: stmt_xio.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h), libcore (eval.h, strings.h, language_descriptor.h)
// Implementation for Atari XIO statement micro-library.
//
// ---- Includes ----

#include "statements/io/device/stmt_xio.h"
#include "device/vdev.h"
#include "device/dev_atari.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_xio_desc = {
    .name = "XIO",
    .category = "System & Hardware",
    .syntax = "XIO cmd, [#]channel, aux1, aux2, \"filespec$\"",
    .description = "Performs extended device I/O control operations (format, lock, unlock, rename, baud rate) across virtual and hardware devices (Atari CIO).",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

void stmt_xio_register(void) {
    lang_desc_register(&g_xio_desc);
}

BppError stmt_xio_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null context in XIO";
        return err;
    }

    // 1. Evaluate cmd (numeric opcode or string name)
    BValue cmd_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int cmd = 0;
    if (cmd_val.type == VAL_NUMBER || cmd_val.type == VAL_INTEGER) {
        cmd = (int)cmd_val.as.number;
    } else if (cmd_val.type == VAL_STRING && cmd_val.as.string) {
        const char *s = str_data(cmd_val.as.string);
        if (!s) s = "";
        if (runtime_strcasecmp(s, "OPEN") == 0) cmd = 3;
        else if (runtime_strcasecmp(s, "GET_RECORD") == 0) cmd = 5;
        else if (runtime_strcasecmp(s, "GET_CHARS") == 0) cmd = 7;
        else if (runtime_strcasecmp(s, "PUT_RECORD") == 0) cmd = 9;
        else if (runtime_strcasecmp(s, "PUT_CHARS") == 0) cmd = 11;
        else if (runtime_strcasecmp(s, "CLOSE") == 0) cmd = 12;
        else if (runtime_strcasecmp(s, "STATUS") == 0) cmd = 13;
        else if (runtime_strcasecmp(s, "RENAME") == 0) cmd = 18;
        else if (runtime_strcasecmp(s, "FORMAT") == 0) cmd = 32;
        else if (runtime_strcasecmp(s, "LOCK") == 0) cmd = 33;
        else if (runtime_strcasecmp(s, "UNLOCK") == 0) cmd = 34;
        else if (runtime_strcasecmp(s, "POINT") == 0 || runtime_strcasecmp(s, "SEEK") == 0) cmd = 35;
        else if (runtime_strcasecmp(s, "NOTE") == 0 || runtime_strcasecmp(s, "TELL") == 0) cmd = 36;
        else if (runtime_strcasecmp(s, "DIRECTORY") == 0) cmd = 38;
        else if (runtime_strcasecmp(s, "DRAWTO") == 0) cmd = 40;
        else if (runtime_strcasecmp(s, "FILL") == 0) cmd = 41;
        else if (runtime_strcasecmp(s, "SOUND") == 0) cmd = 42;
        else if (runtime_strcasecmp(s, "MOTOR_ON") == 0) cmd = 45;
        else if (runtime_strcasecmp(s, "MOTOR_OFF") == 0) cmd = 44;
        else if (runtime_strcasecmp(s, "REWIND") == 0) cmd = 43;
        else if (runtime_strcasecmp(s, "SET_BAUD") == 0) cmd = 70;
        else if (runtime_strcasecmp(s, "TALK") == 0) cmd = 130;
        else if (runtime_strcasecmp(s, "LISTEN") == 0) cmd = 131;
        else if (runtime_strcasecmp(s, "UNLISTEN") == 0) cmd = 132;
        else if (runtime_strcasecmp(s, "UNTALK") == 0) cmd = 133;
        else if (runtime_strcasecmp(s, "CBM_STATUS") == 0) cmd = 135;
        else if (runtime_strcasecmp(s, "CBM_RESET") == 0) cmd = 136;
        else if (runtime_strcasecmp(s, "STEP_IN") == 0) cmd = 140;
        else if (runtime_strcasecmp(s, "STEP_OUT") == 0) cmd = 141;
        else if (runtime_strcasecmp(s, "READ_NIBBLE") == 0) cmd = 143;
        else if (runtime_strcasecmp(s, "WRITE_NIBBLE") == 0) cmd = 144;
        else if (runtime_strcasecmp(s, "SIO_SEND") == 0) cmd = 160;
        else if (runtime_strcasecmp(s, "SIO_STATUS") == 0) cmd = 161;
        else if (runtime_strcasecmp(s, "PBI") == 0) cmd = 164;
        else if (runtime_strcasecmp(s, "ECI") == 0) cmd = 166;
        else if (runtime_strcasecmp(s, "PIN_MODE") == 0) cmd = 200;
        else if (runtime_strcasecmp(s, "DIGITAL_WRITE") == 0) cmd = 201;
        else if (runtime_strcasecmp(s, "DIGITAL_READ") == 0) cmd = 202;
        else if (runtime_strcasecmp(s, "PWM") == 0 || runtime_strcasecmp(s, "ANALOG_WRITE") == 0) cmd = 203;
        else if (runtime_strcasecmp(s, "ADC") == 0 || runtime_strcasecmp(s, "ANALOG_READ") == 0) cmd = 204;
        else if (runtime_strcasecmp(s, "I2C") == 0) cmd = 205;
        else if (runtime_strcasecmp(s, "SPI") == 0) cmd = 206;
        else if (runtime_strcasecmp(s, "DAC") == 0) cmd = 207;
        else if (runtime_strcasecmp(s, "SERVO") == 0) cmd = 208;
        else if (runtime_strcasecmp(s, "NEOPIXEL") == 0) cmd = 209;
        else if (runtime_strcasecmp(s, "SENSOR") == 0) cmd = 210;
        else if (runtime_strcasecmp(s, "RESET") == 0) cmd = 254;
        else cmd = 0;
        str_release(vm_get_str(vm), cmd_val.as.string);
    } else {
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    // Expect comma
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' after command in XIO";
        return err;
    }

    // 2. Optional # and evaluate channel
    BppToken hash = lex_peek(lex);
    if (hash.type == TOK_HASH) {
        lex_next(lex);
    }
    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
        if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    int channel = (int)ch_val.as.number;
    (void)channel;

    // Expect comma
    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' after channel in XIO";
        return err;
    }

    // 3. Evaluate aux1
    BValue aux1_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (aux1_val.type != VAL_NUMBER && aux1_val.type != VAL_INTEGER) {
        if (aux1_val.type == VAL_STRING && aux1_val.as.string) str_release(vm_get_str(vm), aux1_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    int aux1 = (int)aux1_val.as.number;

    // Expect comma
    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' after aux1 in XIO";
        return err;
    }

    // 4. Evaluate aux2
    BValue aux2_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (aux2_val.type != VAL_NUMBER && aux2_val.type != VAL_INTEGER) {
        if (aux2_val.type == VAL_STRING && aux2_val.as.string) str_release(vm_get_str(vm), aux2_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }
    int aux2 = (int)aux2_val.as.number;

    // Expect comma
    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = ERR_SYNTAX;
        err.message = "Expected ',' after aux2 in XIO";
        return err;
    }

    // 5. Evaluate filespec$
    BValue fn_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (fn_val.type != VAL_STRING || !fn_val.as.string) {
        if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
        err.code = ERR_TYPE_MISMATCH;
        return err;
    }

    VDevContext *vctx = vm_get_vdev(vm);
    if (vctx) {
        const char *spec = str_data(fn_val.as.string);
        VDev *dev = vdev_get(vctx, spec);
        if (dev) {
            dev_atari_xio_exec(dev, cmd, aux1, aux2, spec);
            int iargs[4] = {aux1, aux2, 0, 0};
            vdev_ioctl(dev, cmd, iargs);
        }
    }
    str_release(vm_get_str(vm), fn_val.as.string);
    return err;
}
