// FILENAME: ert.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ert.h, string.c)
// Provides runtime implementation for the ERT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/format/ert.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_ert_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ERT$",
        .category = "String Functions",
        .syntax = "ERT$(error_code)",
        .help_text = "Returns the standard error message text corresponding to an error number (DEC BASIC-PLUS).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

static const char *get_error_text(int code) {
    switch (code) {
        case 1: return "NEXT without FOR";
        case 2: return "Syntax error";
        case 3: return "RETURN without GOSUB";
        case 4: return "Out of DATA";
        case 5: return "Illegal function call";
        case 6: return "Overflow";
        case 7: return "Out of memory";
        case 8: return "Undefined line number";
        case 9: return "Subscript out of range";
        case 10: return "Duplicate definition";
        case 11: return "Division by zero";
        case 12: return "Illegal direct";
        case 13: return "Type mismatch";
        case 14: return "Out of string space";
        case 15: return "String formula too complex";
        case 16: return "Cannot continue";
        case 17: return "Undefined user function";
        case 18: return "Undefined user function";
        case 19: return "No RESUME";
        case 20: return "RESUME without error";
        case 24: return "Device timeout";
        case 25: return "Device fault";
        case 27: return "Out of paper";
        case 29: return "WHILE without WEND";
        case 30: return "WEND without WHILE";
        case 50: return "FIELD overflow";
        case 51: return "Internal error";
        case 52: return "Bad file number";
        case 53: return "File not found";
        case 54: return "Bad file mode";
        case 55: return "File already open";
        case 57: return "Device I/O error";
        case 58: return "File already exists";
        case 61: return "Disk full";
        case 62: return "Input past end";
        case 63: return "Bad record number";
        case 64: return "Bad file name";
        case 67: return "Too many files";
        case 68: return "Device unavailable";
        case 70: return "Permission denied";
        case 71: return "Disk not ready";
        case 72: return "Disk media error";
        case 75: return "Path/File access error";
        case 76: return "Path not found";
        default: return "Unknown error";
    }
}

BValue func_ert_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "ERT$ expects one numeric error code";
        return res;
    }

    int code = (int)args[0].as.number;
    const char *msg = get_error_text(code);

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), msg, runtime_strlen(msg));
    return res;
}
